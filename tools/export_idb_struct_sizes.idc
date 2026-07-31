//      Dump every local type's struct size to CSV. Runs inside IDA Free.
//
//      IDA Free 9.4 ships no IDAPython - its plugins directory holds the
//      decompiler and no idapython.so - so File > Script file parses a .py as
//      IDC and dies on the shebang as a bad preprocessor command. This is the
//      IDC version, and the functions it calls were read out of this build's
//      own idc/idc.idc rather than assumed:
//
//          get_ordinal_limit()             line 5276
//          get_numbered_type_name(ord)     line 5301
//          get_struc_id(name)              line 4159
//          get_struc_size(id)              line 4185
//
//      Run it with File > Script file... and it writes the CSV beside this
//      script's output path below, then reports the count in the Output
//      window.
//
//      WHAT COMES OUT IS A HYPOTHESIS, NOT AN ANSWER. Two sources have already
//      failed verification against the 40 classes whose sizes are pinned:
//      Thinker's headers at 5 right and 11 wrong, because its same-named
//      structs are different types, and the other IDB at 31 right and 2 wrong,
//      because some of its structs are incomplete reconstructions. A wrong
//      layout compiles, links, and corrupts memory only at run time. Score this
//      before believing it:
//
//          tools/derive_class_layout.py --score-csv <the csv>

#include <idc.idc>

static main()
{
    auto path, handle, limit, ordinal, name, sid, size, written, skipped;

    path = "/home/fran6co/code/OpenSMACX/.opensmacx/game/idb-struct-sizes.csv";
    handle = fopen(path, "w");
    if (handle == 0)
    {
        msg("cannot write " + path + "\n");
        return;
    }
    fprintf(handle, "struct,size\n");

    limit = get_ordinal_limit();
    written = 0;
    skipped = 0;

    for (ordinal = 1; ordinal < limit; ordinal++)
    {
        name = get_numbered_type_name(ordinal);
        if (name == "" || name == 0)
            continue;

        sid = get_struc_id(name);
        if (sid == -1 || sid == BADADDR)
        {
            // A local type that is not a structure - a typedef, an enum, a
            // function prototype. Counted rather than ignored, so the report
            // says how much of the type library was not a struct.
            skipped = skipped + 1;
            continue;
        }

        size = get_struc_size(sid);
        // A zero size is a forward declaration with no members. Writing it
        // would look exactly like an answer.
        if (size <= 0)
        {
            skipped = skipped + 1;
            continue;
        }

        fprintf(handle, "%s,0x%X\n", name, size);
        written = written + 1;
    }

    fclose(handle);
    msg(sprintf("wrote %d struct size(s) to %s\n", written, path));
    msg(sprintf("%d local type(s) were not sized structures\n", skipped));
    msg("NOT an answer yet - score it:\n");
    msg("  tools/derive_class_layout.py --score-csv " + path + "\n");
}
