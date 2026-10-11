"""Complete CRT section attributions; not linked selection or game acceptance."""
from dataclasses import replace
import csv
import hashlib
import json
import os
from pathlib import Path
import subprocess

import pytest

from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component
from match_recovery.coff_archive import CoffArchive
from match_recovery.import_source import read_imports
from match_recovery.model import Span
from match_recovery.pe_coff import Coff
from test_crt_eight_entry_bindings_oct10 import repin, raw_offset

ROOT = Path(__file__).resolve().parents[2]
COMPANION = "src/vendor/vc6-runtime.recovery.h"
BASE = "d516fb1728a65933c0e4afb8c622f3d862f46b1b"
EXPECTED = (
    (0x00644DD2, 0x00644DFF, "__cinit", "build/intel/mt_obj/crt0dat.obj", 2, "48be3defdca3560f39e7d593e4dd110cb2977874227eae911a707868f722b961"),
    (0x00644DFF, 0x00644E10, "_exit", "build/intel/mt_obj/crt0dat.obj", 4, "48be3defdca3560f39e7d593e4dd110cb2977874227eae911a707868f722b961"),
    (0x00644E10, 0x00644E21, "__exit", "build/intel/mt_obj/crt0dat.obj", 6, "48be3defdca3560f39e7d593e4dd110cb2977874227eae911a707868f722b961"),
    (0x00644F87, 0x00644FD6, "?_UnwindNestedFrames@@YGXPAUEHRegistrationNode@@PAUEHExceptionRecord@@@Z", "build/intel/mt_obj/trnsctrl.obj", 5, "f9efd58c6a13efa62b50d2b9a7532bc77d511fc1d629cbf635f4e6cec6916f9c"),
    (0x0064500C, 0x00645060, "?_CallCatchBlock2@@YAPAXPAUEHRegistrationNode@@PBU_s_FuncInfo@@PAXHK@Z", "build/intel/mt_obj/trnsctrl.obj", 9, "f9efd58c6a13efa62b50d2b9a7532bc77d511fc1d629cbf635f4e6cec6916f9c"),
    (0x00645060, 0x00645085, "?CatchGuardHandler@@YA?AW4_EXCEPTION_DISPOSITION@@PAUEHExceptionRecord@@PAUCatchGuardRN@@PAX2@Z", "build/intel/mt_obj/trnsctrl.obj", 10, "f9efd58c6a13efa62b50d2b9a7532bc77d511fc1d629cbf635f4e6cec6916f9c"),
    (0x00645085, 0x0064513B, "?_CallSETranslator@@YAHPAUEHExceptionRecord@@PAUEHRegistrationNode@@PAX2PBU_s_FuncInfo@@H1@Z", "build/intel/mt_obj/trnsctrl.obj", 11, "f9efd58c6a13efa62b50d2b9a7532bc77d511fc1d629cbf635f4e6cec6916f9c"),
    (0x006451B0, 0x0064522B, "?_GetRangeOfTrysToCheck@@YAPBU_s_TryBlockMapEntry@@PBU_s_FuncInfo@@HHPAI1@Z", "build/intel/mt_obj/trnsctrl.obj", 13, "f9efd58c6a13efa62b50d2b9a7532bc77d511fc1d629cbf635f4e6cec6916f9c"),
    (0x0064531A, 0x00645398, "__onexit", "build/intel/mt_obj/onexit.obj", 2, "88f861d4dbeb26e450537c284442e69e1eceaab59588633656f7c3becdfc303c"),
    (0x00645398, 0x006453AA, "_atexit", "build/intel/mt_obj/onexit.obj", 4, "88f861d4dbeb26e450537c284442e69e1eceaab59588633656f7c3becdfc303c"),
    (0x006455C9, 0x00645615, "__fclose_lk", "build/intel/mt_obj/fclose.obj", 3, "402d0d62cc3b2c85176676826ec61f0f4b87c11a9190bde109b575839514f895"),
    (0x0064584D, 0x00645864, "__fpmath", "../build/intel/mt_obj/fpinit.obj", 3, "feeea88b943dc112e48e44ebad2cfc6ff610ac1053346f6a7f27a506a0f35982"),
    (0x00645864, 0x00645865, "__fpclear", "../build/intel/mt_obj/fpinit.obj", 5, "feeea88b943dc112e48e44ebad2cfc6ff610ac1053346f6a7f27a506a0f35982"),
    (0x00645865, 0x0064589D, "__cfltcvt_init", "../build/intel/mt_obj/fpinit.obj", 7, "feeea88b943dc112e48e44ebad2cfc6ff610ac1053346f6a7f27a506a0f35982"),
    (0x0064628F, 0x006462FE, "_toupper", "build/intel/mt_obj/toupper.obj", 3, "fa79d0298553e5e59cf52f65e7a13666dfde4f5df9cb36ececd792e82371f39d"),
    (0x006462FE, 0x006463CA, "__toupper_lk", "build/intel/mt_obj/toupper.obj", 5, "fa79d0298553e5e59cf52f65e7a13666dfde4f5df9cb36ececd792e82371f39d"),
    (0x006466EF, 0x0064675E, "_tolower", "build/intel/mt_obj/tolower.obj", 3, "f0ff862df6b236cdb66ea27a28d19289804844e78332695780c1c6b299c974ae"),
    (0x0064675E, 0x00646829, "__tolower_lk", "build/intel/mt_obj/tolower.obj", 5, "f0ff862df6b236cdb66ea27a28d19289804844e78332695780c1c6b299c974ae"),
    (0x00646855, 0x00646886, "_fgetc", "build/intel/mt_obj/fgetc.obj", 1, "e796a81eb4f21101b7bd3de77ac517cd8f5b5f99ec9e26e0545c4db748cc920b"),
    (0x00646886, 0x006468C1, "_fputc", "build/intel/mt_obj/fputc.obj", 1, "5be329c65b5146a5b75625f490980426f009d8d29dc04cb34eba00037d06843e"),
    (0x006468D0, 0x00646950, "_strstr", "build/intel/mt_obj/strstr.obj", 1, "f9921fa209d823ec461f0a8374c623b4ea082da49aba3d4bef1f2a98c8cc7653"),
    (0x0064697A, 0x006469A2, "_isupper", "build/intel/mt_obj/_ctype.obj", 3, "3faa12b6e7fea8e294e045d0c28c66cf6d186e1874ac6721e2012515e1387138"),
    (0x006469A2, 0x006469CA, "_isdigit", "build/intel/mt_obj/_ctype.obj", 7, "3faa12b6e7fea8e294e045d0c28c66cf6d186e1874ac6721e2012515e1387138"),
    (0x006469CA, 0x006469F7, "_isxdigit", "build/intel/mt_obj/_ctype.obj", 9, "3faa12b6e7fea8e294e045d0c28c66cf6d186e1874ac6721e2012515e1387138"),
    (0x00646A25, 0x00646A77, "_sprintf", "build/intel/mt_obj/sprintf.obj", 1, "5e94947d6847f43008f767e804d4b38c55e09ff3e4608c52df4234e8c6d57dd3"),
    (0x00646F5D, 0x00646FD4, "__beginthread", "build/intel/mt_obj/thread.obj", 1, "5cce4c839d1764f275e326b85b388f98e40622a9f2f2030843082c9023b2e596"),
    (0x006470A6, 0x006470B8, "_malloc", "build/intel/mt_obj/malloc.obj", 1, "7b835e6ee94d5f05ce10cff4d9941b9ac0d91e681802c5f88f51a72d743bfe65"),
    (0x006470B8, 0x006470E4, "__nh_malloc", "build/intel/mt_obj/malloc.obj", 3, "7b835e6ee94d5f05ce10cff4d9941b9ac0d91e681802c5f88f51a72d743bfe65"),
    (0x006470E4, 0x00647132, "__heap_alloc", "build/intel/mt_obj/malloc.obj", 5, "7b835e6ee94d5f05ce10cff4d9941b9ac0d91e681802c5f88f51a72d743bfe65"),
    (0x00647132, 0x0064726A, "_realloc", "build/intel/mt_obj/realloc.obj", 1, "689f5777ccb33f5542e0f292b5139eac276011b2e382f1adf5925bb36d6526a8"),
    (0x0064726A, 0x006472CC, "_fgets", "build/intel/mt_obj/fgets.obj", 1, "21d440cd59da36b22d7d235fba0b11e6fa8793003976128800b433521bc5dd61"),
    (0x006472CC, 0x00647330, "_rewind", "build/intel/mt_obj/rewind.obj", 1, "933bb463bb5366210890830175fb7a35440a600162b3ae687763554fbfedb70b"),
    (0x0064735C, 0x006473E9, "__fseek_lk", "build/intel/mt_obj/fseek.obj", 2, "54ea196d149e1362aa423d8039060e7f0009b75c39187a0fc09be3d0d9f373f2"),
    (0x00647851, 0x00647920, "_floor", "../build/intel/mt_obj/floor.obj", 2, "fc52dc5aff19358916c05104bbffaf092b08061cbb8b23266be6556f171cc925"),
    (0x00647942, 0x00647AA3, "__ftell_lk", "build/intel/mt_obj/ftell.obj", 3, "8b4c31d5b987fcbab1a9c5b7e83720995bdce825b701262ae7cb0910e2fe4b8f"),
    (0x00647CDB, 0x00647D04, "__mtinitlocks", "build/intel/mt_obj/mlock.obj", 3, "d5041da3ffbc98943299b7078cc9498de1dbd57d3eef552598494042d23be735"),
    (0x006488F5, 0x00648990, "___InternalCxxFrameHandler", "build/intel/mt_obj/frame.obj", 2, "2b87b6fa3f780e75aa5c34ed7a8091416460a13053c1ebff2593df350396c934"),
    (0x00648B43, 0x00648BED, "?FindHandlerForForeignException@@YAXPAUEHExceptionRecord@@PAUEHRegistrationNode@@PAU_CONTEXT@@PAXPBU_s_FuncInfo@@HH1@Z", "build/intel/mt_obj/frame.obj", 4, "2b87b6fa3f780e75aa5c34ed7a8091416460a13053c1ebff2593df350396c934"),
    (0x00648CFE, 0x00648D79, "?CatchIt@@YAXPAUEHExceptionRecord@@PAUEHRegistrationNode@@PAU_CONTEXT@@PAXPBU_s_FuncInfo@@PBU_s_HandlerType@@PBU_s_CatchableType@@PBU_s_TryBlockMapEntry@@H1E@Z", "build/intel/mt_obj/frame.obj", 11, "2b87b6fa3f780e75aa5c34ed7a8091416460a13053c1ebff2593df350396c934"),
    (0x0064915C, 0x006491B0, "__mtinit", "build/intel/mt_obj/tidtable.obj", 2, "f9f509000e8996dd877fc89a0f9e2f92f22da4c3bdbd229557a45707e5ca371f"),
    (0x00649381, 0x006493C6, "__msize", "build/intel/mt_obj/msize.obj", 1, "0380b7114d147d0ee44395cba0984b08fe14d18afe3c9ee38bc78e3a236a2ec1"),
    (0x006493C6, 0x00649423, "__close", "build/intel/mt_obj/close.obj", 1, "3aca8f676f71830f2fd49e54ab3b985db187a25a42d1d838fda9d2a4d9cae2e4"),
    (0x00649423, 0x006494A6, "__close_lk", "build/intel/mt_obj/close.obj", 3, "3aca8f676f71830f2fd49e54ab3b985db187a25a42d1d838fda9d2a4d9cae2e4"),
    (0x00649608, 0x00649778, "__openfile", "build/intel/mt_obj/_open.obj", 1, "a61c9d640b24d28507e1291bad657d73442e82d47834dac16dfc690952ac5ef0"),
    (0x00649778, 0x00649840, "__getstream", "build/intel/mt_obj/stream.obj", 1, "cc7621851151daccbac671b95c050b259bee6a8a90c9e70c81769308305c504f"),
    (0x00649852, 0x00649890, "__ms_p5_test_fdiv", "../build/intel/mt_obj/testfdiv.obj", 1, "a0598b6342ad36b2c58060a8837006c97ab4c58ca680fca2c215558eea72037b"),
    (0x006498B9, 0x00649913, "__forcdecpt", "../build/intel/mt_obj/cvt.obj", 1, "b222cce7bc13671f976e1a77a876d39399b1225cdac96d67740167407358b7c2"),
    (0x006499B7, 0x00649A18, "__cftoe", "../build/intel/mt_obj/cvt.obj", 9, "b222cce7bc13671f976e1a77a876d39399b1225cdac96d67740167407358b7c2"),
    (0x00649A18, 0x00649ADA, "__cftoe2", "../build/intel/mt_obj/cvt.obj", 10, "b222cce7bc13671f976e1a77a876d39399b1225cdac96d67740167407358b7c2"),
    (0x00649ADA, 0x00649B2F, "__cftof", "../build/intel/mt_obj/cvt.obj", 12, "b222cce7bc13671f976e1a77a876d39399b1225cdac96d67740167407358b7c2"),
    (0x00649B2F, 0x00649BD6, "__cftof2", "../build/intel/mt_obj/cvt.obj", 13, "b222cce7bc13671f976e1a77a876d39399b1225cdac96d67740167407358b7c2"),
    (0x00649BD6, 0x00649C69, "__cftog", "../build/intel/mt_obj/cvt.obj", 14, "b222cce7bc13671f976e1a77a876d39399b1225cdac96d67740167407358b7c2"),
    (0x00649C69, 0x00649CBA, "__cfltcvt", "../build/intel/mt_obj/cvt.obj", 15, "b222cce7bc13671f976e1a77a876d39399b1225cdac96d67740167407358b7c2"),
    (0x00649CBA, 0x00649CDF, "__shift", "../build/intel/mt_obj/cvt.obj", 16, "b222cce7bc13671f976e1a77a876d39399b1225cdac96d67740167407358b7c2"),
    (0x00649CDF, 0x00649D54, "__isctype", "build/intel/mt_obj/isctype.obj", 1, "0ea9d0e147d9c22e840315e0f07d6245b31a80e0eb489d61648afe55ade3325f"),
    (0x0064A09C, 0x0064A178, "__filbuf", "build/intel/mt_obj/_filbuf.obj", 1, "090795e5e40c1b5bab723fea838cd7a2b649a205288223f31e243152decb9666"),
    (0x0064A1DD, 0x0064A3B6, "__read_lk", "build/intel/mt_obj/read.obj", 3, "e8ff059f905a1d2f1fb1598354e5ffa7dbdf349bcaf29cfc2c00c8f930f28be3"),
    (0x0064A9C3, 0x0064AA36, "__dosmaperr", "build/intel/mt_obj/dosmap.obj", 2, "687d2b49e9b78f360c6dd2cd137b89ec11dd446c022c3b8729d72296365f550a"),
    (0x0064B189, 0x0064B1BE, "_write_char", "build/intel/mt_obj/output.obj", 6, "aaabeb2d6637972ea62f282394b45972ec6e6bdb3ac31f590da1acc7d046cf29"),
    (0x0064B1EF, 0x0064B227, "_write_string", "build/intel/mt_obj/output.obj", 9, "aaabeb2d6637972ea62f282394b45972ec6e6bdb3ac31f590da1acc7d046cf29"),
    (0x0064B467, 0x0064B597, "__powhlp", "../build/intel/mt_obj/powhlp.obj", 1, "e73109da264a3a8d5e1ee03452cbb3f8f3df9832a5e9a0cae65c7553278d9bdf"),
    (0x0064B597, 0x0064B5FC, "__d_inttype", "../build/intel/mt_obj/powhlp.obj", 4, "e73109da264a3a8d5e1ee03452cbb3f8f3df9832a5e9a0cae65c7553278d9bdf"),
    (0x0064B5FC, 0x0064B73A, "__XcptFilter", "build/intel/mt_obj/winxfltr.obj", 2, "0404128952dcc324d73be474eefbbcab0f46d9ea673ed765ebd40382ccb9a3ba"),
    (0x0064B73A, 0x0064B774, "_xcptlookup", "build/intel/mt_obj/winxfltr.obj", 3, "0404128952dcc324d73be474eefbbcab0f46d9ea673ed765ebd40382ccb9a3ba"),
    (0x0064B774, 0x0064B7CC, "__wincmdln", "build/intel/mt_obj/wincmdln.obj", 1, "34401103a87f33657fca556f0644d883272b8c48296fe5c350f9077ec7ce9c6c"),
    (0x0064B7CC, 0x0064B885, "__setenvp", "build/intel/mt_obj/stdenvp.obj", 1, "3c2875fc8a93fd49ae04d09b6c4b68e189fe1b8f4cb69da759f5573e34f246e4"),
    (0x0064B885, 0x0064B91E, "__setargv", "build/intel/mt_obj/stdargv.obj", 1, "0fcb2efe177d1044e94174ed0cdb0fb69fe28fdf55a96d8a337cf874e85c6382"),
    (0x0064B91E, 0x0064BAD2, "_parse_cmdline", "build/intel/mt_obj/stdargv.obj", 3, "0fcb2efe177d1044e94174ed0cdb0fb69fe28fdf55a96d8a337cf874e85c6382"),
    (0x0064BAD2, 0x0064BC04, "___crtGetEnvironmentStringsA", "build/intel/mt_obj/a_env.obj", 2, "9fbb6e694fb5a0e9924796dd8c8e840af6ab423e95b4bb1d2ef5b197095f05c2"),
    (0x0064BC04, 0x0064BDC0, "__ioinit", "build/intel/mt_obj/ioinit.obj", 2, "9b38b25570afe2d706cf2390ab55773197687f79e09ad52372cdf9baf9d13c3c"),
    (0x0064BDC0, 0x0064BDF9, "__FF_MSGBANNER", "build/intel/mt_obj/crt0msg.obj", 21, "245e7a87cbf041961bd3459aca866f938a3cd533a997464c492c420328fb1dac"),
    (0x0064BDF9, 0x0064BF4C, "__NMSG_WRITE", "build/intel/mt_obj/crt0msg.obj", 23, "245e7a87cbf041961bd3459aca866f938a3cd533a997464c492c420328fb1dac"),
    (0x0064BF4C, 0x0064BF67, "__callnewh", "build/intel/mt_obj/handler.obj", 6, "b471b8aa01041974c43bbdba356dc5fe91e94a2aa490d94d08a89cf38ca42d0d"),
    (0x0064BFCC, 0x0064C03F, "__lseek_lk", "build/intel/mt_obj/lseek.obj", 3, "d0e995a8077188a06229360a13286e2618ae971c0a0e657447c415469271e153"),
    (0x0064C03F, 0x0064C0CC, "__stbuf", "build/intel/mt_obj/_sftbuf.obj", 2, "674b8654188a4ba2c232fc34bda4f60a16787c91cfed8139ad27ac744f5dab76"),
    (0x0064C0CC, 0x0064C0F6, "__ftbuf", "build/intel/mt_obj/_sftbuf.obj", 4, "674b8654188a4ba2c232fc34bda4f60a16787c91cfed8139ad27ac744f5dab76"),
    (0x0064C0F6, 0x0064C14A, "__handle_qnan1", "../build/intel/mt_obj/fpexcept.obj", 29, "e03c9af2bffd6b54ee6b44a5766fcc787e670459b84046f92e972b5985b767e4"),
    (0x0064C14A, 0x0064C1AA, "__handle_qnan2", "../build/intel/mt_obj/fpexcept.obj", 30, "e03c9af2bffd6b54ee6b44a5766fcc787e670459b84046f92e972b5985b767e4"),
    (0x0064C1AA, 0x0064C242, "__except1", "../build/intel/mt_obj/fpexcept.obj", 31, "e03c9af2bffd6b54ee6b44a5766fcc787e670459b84046f92e972b5985b767e4"),
    (0x0064C242, 0x0064C2E7, "__except2", "../build/intel/mt_obj/fpexcept.obj", 32, "e03c9af2bffd6b54ee6b44a5766fcc787e670459b84046f92e972b5985b767e4"),
    (0x0064C2E7, 0x0064C59A, "__raise_exc", "../build/intel/mt_obj/fpexcept.obj", 33, "e03c9af2bffd6b54ee6b44a5766fcc787e670459b84046f92e972b5985b767e4"),
    (0x0064C59A, 0x0064C7B1, "__handle_exc", "../build/intel/mt_obj/fpexcept.obj", 34, "e03c9af2bffd6b54ee6b44a5766fcc787e670459b84046f92e972b5985b767e4"),
    (0x0064C7B1, 0x0064C839, "__umatherr", "../build/intel/mt_obj/fpexcept.obj", 36, "e03c9af2bffd6b54ee6b44a5766fcc787e670459b84046f92e972b5985b767e4"),
    (0x0064C861, 0x0064C886, "__get_fname", "../build/intel/mt_obj/fpexcept.obj", 39, "e03c9af2bffd6b54ee6b44a5766fcc787e670459b84046f92e972b5985b767e4"),
    (0x0064C95C, 0x0064CA1D, "__decomp", "../build/intel/mt_obj/util.obj", 9, "69e809144b016a7b208002dde6a8c5407a9f61a336897f167c3f3c9b3573fc95"),
    (0x0064CA5D, 0x0064CAB3, "__set_statfp", "../build/intel/mt_obj/fpctrl.obj", 5, "14b7ee6b04bf7cfcd33510957e24e2e6682ebafab71fac7be62f2a2d64de84c7"),
    (0x0064CB34, 0x0064CB7A, "?__CxxUnhandledExceptionFilter@@YGJPAU_EXCEPTION_POINTERS@@@Z", "build/intel/mt_obj/unhandld.obj", 3, "198280f1f19e4350f7d28346a8a66eed0b063dd4adec776044ae3fa3c723e6b4"),
    (0x0064CBD0, 0x0064CBE8, "?_ValidateExecute@@YAHP6GHXZ@Z", "build/intel/mt_obj/validate.obj", 5, "a12a6559529c363ab497ad3f717b9d4fa79252541a181e2d46dc716197b5a0a7"),
    (0x0064CBFF, 0x0064CD22, "__alloc_osfhnd", "build/intel/mt_obj/osfinfo.obj", 1, "95bc91e895163d1d9028ab03ae0bd1c3b25140ce3638f47af14d993d8f929677"),
    (0x0064CD22, 0x0064CD9E, "__set_osfhnd", "build/intel/mt_obj/osfinfo.obj", 3, "95bc91e895163d1d9028ab03ae0bd1c3b25140ce3638f47af14d993d8f929677"),
    (0x0064CD9E, 0x0064CE1D, "__free_osfhnd", "build/intel/mt_obj/osfinfo.obj", 5, "95bc91e895163d1d9028ab03ae0bd1c3b25140ce3638f47af14d993d8f929677"),
    (0x0064CE1D, 0x0064CE5F, "__get_osfhandle", "build/intel/mt_obj/osfinfo.obj", 7, "95bc91e895163d1d9028ab03ae0bd1c3b25140ce3638f47af14d993d8f929677"),
    (0x0064CE5F, 0x0064CEBE, "__lock_fhandle", "build/intel/mt_obj/osfinfo.obj", 10, "95bc91e895163d1d9028ab03ae0bd1c3b25140ce3638f47af14d993d8f929677"),
    (0x0064CEBE, 0x0064CEE0, "__unlock_fhandle", "build/intel/mt_obj/osfinfo.obj", 12, "95bc91e895163d1d9028ab03ae0bd1c3b25140ce3638f47af14d993d8f929677"),
    (0x0064CEE0, 0x0064CF73, "__commit", "build/intel/mt_obj/commit.obj", 1, "65a4ed0eda4ea0c010300b5207e76e17ec772ae7ffb5102d5ba69824838c5f41"),
    (0x0064CF8A, 0x0064D259, "__sopen", "build/intel/mt_obj/open.obj", 3, "757581b80e90ffc823121b7120e1436cc26ca8ea1e8f4f79f1070533d493985a"),
    (0x0064D7AB, 0x0064D822, "__fptostr", "build/intel/mt_obj/_fptostr.obj", 1, "83d7109092fe28971bfa5a074f25178fc5dc825775ed31f1c6a01931ee86b1fb"),
    (0x0064D822, 0x0064D87E, "__fltout2", "../build/intel/mt_obj/cfout.obj", 1, "da8583e62749a413c76d64adef8e6c317a61dabad19462bf0337b7d7d4b449ff"),
    (0x0064DA86, 0x0064DACA, "__getbuf", "build/intel/mt_obj/_getbuf.obj", 1, "b02b48a921c93d331a100d7da29ff03e34bdadc33b9a2602bc23976de54f1899"),
    (0x0064DACA, 0x0064DAF3, "__isatty", "build/intel/mt_obj/isatty.obj", 1, "19fa34d81108b31043eadfe11ba5f4e5f2e54a817cf6186e99b6119a0e32d2f2"),
    (0x0064DB3A, 0x0064DC0A, "__87except", "../build/intel/mt_obj/87except.obj", 1, "98f97b62efa50aae290877de75712bb82a63241c725269996d312734717441e5"),
    (0x0064DC0A, 0x0064DC63, "_wctomb", "build/intel/mt_obj/wctomb.obj", 1, "4412763dc866a2852fe78494d97393aec454130df9c4a2304e89911c4c868ee4"),
    (0x0064DC63, 0x0064DCCC, "__wctomb_lk", "build/intel/mt_obj/wctomb.obj", 2, "4412763dc866a2852fe78494d97393aec454130df9c4a2304e89911c4c868ee4"),
    (0x0064EB4B, 0x0064EBDD, "__fpclass", "../build/intel/mt_obj/ieeemisc.obj", 13, "bacc7a16ae0471438a73dc2dbea6310e1b0741c84cf6493c6ff309ce7cbaf8cb"),
    (0x0064EBDD, 0x0064EBEE, "__ismbblead", "build/intel/mt_obj/ismbbyte.obj", 17, "a82d6c6e4cf1f7a780f7f5ffdf13c4bdd4d5c05615dd1817147ba42470fb3d64"),
    (0x0064EBEE, 0x0064EC1F, "_x_ismbbtype", "build/intel/mt_obj/ismbbyte.obj", 23, "a82d6c6e4cf1f7a780f7f5ffdf13c4bdd4d5c05615dd1817147ba42470fb3d64"),
    (0x0064EC1F, 0x0064EDCC, "__setmbcp", "build/intel/mt_obj/mbctype.obj", 3, "74924cfef4395b03a742c0dbcbff475c3ed322e1853cedec84c8e28a48a0874e"),
    (0x0064EDCC, 0x0064EE16, "_getSystemCP", "build/intel/mt_obj/mbctype.obj", 5, "74924cfef4395b03a742c0dbcbff475c3ed322e1853cedec84c8e28a48a0874e"),
    (0x0064EE49, 0x0064EE72, "_setSBCS", "build/intel/mt_obj/mbctype.obj", 9, "74924cfef4395b03a742c0dbcbff475c3ed322e1853cedec84c8e28a48a0874e"),
    (0x0064EE72, 0x0064EFF7, "_setSBUpLow", "build/intel/mt_obj/mbctype.obj", 11, "74924cfef4395b03a742c0dbcbff475c3ed322e1853cedec84c8e28a48a0874e"),
    (0x0064EFF7, 0x0064F013, "___initmbctable", "build/intel/mt_obj/mbctype.obj", 14, "74924cfef4395b03a742c0dbcbff475c3ed322e1853cedec84c8e28a48a0874e"),
    (0x0064F09F, 0x0064F221, "_raise", "build/intel/mt_obj/winsig.obj", 5, "dd51b5f10166bdbd9e20978c34f726adbc9d7600c3317d9515d7b7de62a8d8d0"),
    (0x0064F221, 0x0064F25E, "_siglookup", "build/intel/mt_obj/winsig.obj", 6, "dd51b5f10166bdbd9e20978c34f726adbc9d7600c3317d9515d7b7de62a8d8d0"),
    (0x0064F25E, 0x0064F383, "__chsize_lk", "build/intel/mt_obj/chsize.obj", 3, "3202615e060a022aae0eb6d9b698326bb45a3d02af1fa52a097fab5efde0c718"),
    (0x0064FDF0, 0x0064FEF1, "__strnicmp", "build/intel/mt_obj/strnicmp.obj", 1, "8ffc8d9c666478fe6c93b65b74c2506b844835d9ea535264332c6b3116a699a8"),
    (0x0064FEF1, 0x0064FF52, "__setmode_lk", "build/intel/mt_obj/setmode.obj", 3, "c9878f78011b6cb8ea9fe50c4b943766078d75d844ed9bf38838c364bbaa649c"),
    (0x00650172, 0x006501EE, "___multtenpow12", "../build/intel/mt_obj/tenpow.obj", 2, "a6bbb4829116d2aef96d81de0fb3369a7668e2e961f5808d8803d66a6597ce24"),
    (0x00650231, 0x0065024E, "__hypot", "../build/intel/mt_obj/hypot.obj", 1, "54b66efa37aae8d2b59f21629c457ca1c033de7912631ecce7bf38d99486490e"),
    (0x0065024E, 0x006504D3, "__hypothlp", "../build/intel/mt_obj/hypot.obj", 5, "54b66efa37aae8d2b59f21629c457ca1c033de7912631ecce7bf38d99486490e"),
)
ADDRESSES = {x[0] for x in EXPECTED}


def prior_text():
    return subprocess.check_output(["git", "show", BASE + ":" + COMPANION],
                                   cwd=ROOT, text=True)


def test_only_expected_bindings_added():
    old = prior_text()
    current = (ROOT / COMPANION).read_text()
    before, after = parse_component(old), parse_component(current)
    assert len(before.bindings) == 105 and len(after.bindings) == 224
    assert replace(after, bindings=tuple(b for b in after.bindings
                   if b.origin.address not in ADDRESSES)) == before
    additions = [b for b in after.bindings if b.origin.address in ADDRESSES]
    assert [(b.origin.address, b.spans[0].end, b.symbol, b.unit) for b in additions] == [
        x[:4] for x in EXPECTED]
    lines = [line for line in current.splitlines()
             if line.startswith("//     {") and '"unit"' in line
             and int(json.loads(line[3:].rstrip(","))["origin"], 16) in ADDRESSES]
    assert len(lines) == 119
    for line in lines:
        current = current.replace(line + "\n", "", 1)
    last = next(line for line in reversed(old.splitlines()) if '"unit"' in line and '"origin"' in line)
    assert current.replace(last + ",\n", last + "\n", 1) == old


@pytest.fixture(scope="module")
def inputs():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    archive_path = Path("/home/fran6co/opt/vc6/LIB/LIBCMT.LIB")
    if not path.is_file() or not archive_path.is_file():
        pytest.skip("external pinned image/runtime archive unavailable")
    manifest = parse_component((ROOT / COMPANION).read_text())
    image, archive = path.read_bytes(), archive_path.read_bytes()
    assert hashlib.sha256(image).hexdigest() == manifest.bindings[0].origin.image_sha256
    assert hashlib.sha256(archive).hexdigest() == manifest.archive_sha256
    return manifest, image, archive, read_imports(ROOT)


def verify(inputs, manifest=None, image=None):
    m, i, archive, imports = inputs
    return verify_toolchain(m if manifest is None else manifest,
                            i if image is None else image, archive,
                            import_bindings=imports)


def row(report, address):
    return next(r for r in report["functions"] if int(r["address"], 16) == address)


def test_catalogue_extents_and_physical_archive_members(inputs):
    _, _, archive, _ = inputs
    members = CoffArchive(archive)
    with (ROOT / "docs/recovery/ghidra-functions.csv").open() as stream:
        catalogue = {int(r["address"], 16): r for r in csv.DictReader(stream)}
    for address, end, symbol, unit, section, digest in EXPECTED:
        assert int(catalogue[address]["end_address"], 16) == end
        data = members.member(unit)
        assert hashlib.sha256(data).hexdigest() == digest
        obj = Coff(data)
        function = obj.function(symbol, comdat=False)
        assert function.section == section and function.value == 0
        assert len(obj.subject(function)[0]) == end - address


def test_complete_bodies_and_preserved_known_references(inputs):
    manifest, _, _, _ = inputs
    before = verify(inputs, parse_component(prior_text()))
    after = verify(inputs)
    assert len(after["functions"]) == 224
    for address in ADDRESSES:
        r = row(after, address)
        assert r["boundary"] == "pass" and not r["differing_offsets"]
        assert r["result"] in ("pass", "unknown")
        assert all(x["result"] in ("pass", "unknown") for x in r["relocations"])
    for old in before["functions"]:
        current = row(after, int(old["address"], 16))
        if old == current:
            continue
        for key in ("boundary", "differing_offsets"):
            assert current[key] == old[key]
        assert current["bytes"] == old["bytes"] or (
            old["bytes"] == "unknown" and current["bytes"] == "pass")
        if old["result"] == "pass":
            assert current["result"] == "pass"
        assert current["result"] != "fail"
        assert len(old["relocations"]) == len(current["relocations"])
        for a, b in zip(old["relocations"], current["relocations"]):
            assert a["symbol"] == b["symbol"]
            if a["result"] == "pass":
                assert b["result"] == "pass"
            if a != b:
                assert a["result"] == "unknown" and b["result"] == "pass"
                assert a["symbol"] in {x[2] for x in EXPECTED}
    assert after["result"] == "unknown"
    assert after["application_recoveries"] == 0
    assert not after["ready_for_collection"]


@pytest.mark.parametrize("address,end,symbol,unit,section,digest", EXPECTED)
def test_opcode_damage_detected(inputs, address, end, symbol, unit, section, digest):
    manifest, image, _, _ = inputs
    damaged = bytearray(image)
    damaged[raw_offset(image, address)] ^= 1
    damaged = bytes(damaged)
    assert row(verify(inputs, repin(manifest, damaged), damaged), address)["result"] == "fail"


@pytest.mark.parametrize("address,end,symbol,unit,section,digest", EXPECTED)
def test_shortened_boundary_refused(inputs, address, end, symbol, unit, section, digest):
    manifest, _, _, _ = inputs
    if end - address == 1:
        with pytest.raises(ValueError, match="nonempty"):
            Span(address, end - 1)
        return
    changed = replace(manifest, bindings=tuple(
        replace(b, spans=(Span(address, end - 1),)) if b.origin.address == address else b
        for b in manifest.bindings))
    r = row(verify(inputs, changed), address)
    assert r["boundary"] != "pass" and r["result"] != "pass"

