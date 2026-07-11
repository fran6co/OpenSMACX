// Export function metadata for tools/correlate_recovery_analyses.py.
// @category OpenSMACX

import ghidra.app.script.GhidraScript;
import ghidra.framework.Application;
import ghidra.program.model.address.AddressSetView;
import ghidra.program.model.address.AddressRange;
import ghidra.program.model.address.AddressRangeIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;

import java.io.BufferedWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class ExportFunctions extends GhidraScript {
    private static String csv(String value) {
        if (value == null) {
            return "";
        }
        return "\"" + value.replace("\"", "\"\"")
            .replace("\r", "")
            .replace("\n", "\\n") + "\"";
    }

    @Override
    public void run() throws Exception {
        String[] arguments = getScriptArgs();
        if (arguments.length != 1) {
            throw new IllegalArgumentException("expected output CSV path");
        }

        Path output = Paths.get(arguments[0]);
        Path parent = output.toAbsolutePath().getParent();
        if (parent != null) {
            Files.createDirectories(parent);
        }

        String executableHash = currentProgram.getExecutableSHA256();
        String ghidraVersion = Application.getApplicationVersion();
        String language = currentProgram.getLanguageID().getIdAsString();
        int count = 0;
        try (BufferedWriter writer = Files.newBufferedWriter(
                output, StandardCharsets.UTF_8)) {
            writer.write("program_sha256,ghidra_version,language,address,end_address,size,"
                + "body_ranges,name,signature,calling_convention,is_thunk\n");
            FunctionIterator functions = currentProgram.getFunctionManager().getFunctions(true);
            while (functions.hasNext()) {
                Function function = functions.next();
                if (function.isExternal() || !function.getEntryPoint().isMemoryAddress()) {
                    continue;
                }
                AddressSetView body = function.getBody();
                long start = function.getEntryPoint().getOffset();
                long end = start;
                StringBuilder ranges = new StringBuilder();
                AddressRangeIterator rangeIterator = body.getAddressRanges(true);
                while (rangeIterator.hasNext()) {
                    AddressRange range = rangeIterator.next();
                    long rangeStart = range.getMinAddress().getOffset();
                    long rangeEnd = range.getMaxAddress().getOffset() + 1;
                    if (ranges.length() > 0) {
                        ranges.append(';');
                    }
                    ranges.append(String.format("0x%08X-0x%08X", rangeStart, rangeEnd));
                    if (range.contains(function.getEntryPoint())) {
                        end = rangeEnd;
                    }
                }
                if (end == start) {
                    throw new IllegalStateException(
                        "function body does not contain entry point: " + function.getEntryPoint());
                }
                writer.write(String.join(",",
                    executableHash,
                    ghidraVersion,
                    language,
                    String.format("0x%08X", start),
                    String.format("0x%08X", end),
                    Long.toString(body.getNumAddresses()),
                    csv(ranges.toString()),
                    csv(function.getName(true)),
                    csv(function.getPrototypeString(true, true)),
                    function.getCallingConventionName(),
                    Boolean.toString(function.isThunk())));
                writer.write("\n");
                count++;
            }
        }
        println("Exported " + count + " Ghidra functions to " + output);
    }
}
