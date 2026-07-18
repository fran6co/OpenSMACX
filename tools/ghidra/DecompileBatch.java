// Decompile a list of function addresses into a directory with a manifest.
// @category OpenSMACX

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

public class DecompileBatch extends GhidraScript {
    private static String escape(String value) {
        StringBuilder builder = new StringBuilder();
        for (int index = 0; index < value.length(); index++) {
            char character = value.charAt(index);
            if (character == '"' || character == '\\') {
                builder.append('\\');
                builder.append(character);
            } else if (character < 0x20) {
                builder.append(String.format("\\u%04x", (int) character));
            } else {
                builder.append(character);
            }
        }
        return builder.toString();
    }

    private static String sanitize(String name) {
        return name.replaceAll("[^A-Za-z0-9_.-]", "-");
    }

    @Override
    public void run() throws Exception {
        String[] arguments = getScriptArgs();
        if (arguments.length != 3) {
            throw new IllegalArgumentException(
                "expected address list path, output directory, and timeout seconds");
        }
        Path addressList = Paths.get(arguments[0]);
        Path outputDirectory = Paths.get(arguments[1]);
        int timeoutSeconds = Integer.parseInt(arguments[2]);
        Files.createDirectories(outputDirectory);

        List<String> entries = new ArrayList<>();
        DecompInterface decompiler = new DecompInterface();
        try {
            if (!decompiler.openProgram(currentProgram)) {
                throw new IllegalStateException(decompiler.getLastMessage());
            }
            for (String line : Files.readAllLines(addressList, StandardCharsets.UTF_8)) {
                String trimmed = line.trim();
                if (trimmed.isEmpty()) {
                    continue;
                }
                String entry;
                try {
                    Address address = toAddr(trimmed);
                    Function function = getFunctionAt(address);
                    if (function == null) {
                        // The canonical inventory is authoritative for entry
                        // points; materialize entries Ghidra's auto-analysis
                        // merged away so the canonical address decompiles.
                        if (getInstructionAt(address) == null) {
                            disassemble(address);
                        }
                        function = createFunction(address, null);
                    }
                    if (function == null) {
                        throw new IllegalArgumentException(
                            "function entry not found at " + trimmed);
                    }
                    DecompileResults results =
                        decompiler.decompileFunction(function, timeoutSeconds, monitor);
                    if (!results.decompileCompleted()) {
                        throw new IllegalStateException(results.getErrorMessage());
                    }
                    String fileName = trimmed + "-"
                        + sanitize(function.getName(true)) + ".c";
                    Files.writeString(
                        outputDirectory.resolve(fileName),
                        results.getDecompiledFunction().getC(),
                        StandardCharsets.UTF_8);
                    entry = "  \"" + escape(trimmed) + "\": {\"status\": \"ok\", "
                        + "\"name\": \"" + escape(function.getName(true)) + "\", "
                        + "\"file\": \"" + escape(fileName) + "\"}";
                } catch (Exception error) {
                    entry = "  \"" + escape(trimmed) + "\": {\"status\": \"error\", "
                        + "\"error\": \"" + escape(String.valueOf(error.getMessage()))
                        + "\"}";
                }
                entries.add(entry);
            }
        }
        finally {
            decompiler.dispose();
        }
        String manifest = "{\n" + String.join(",\n", entries) + "\n}\n";
        Files.writeString(
            outputDirectory.resolve("run-manifest.json"), manifest,
            StandardCharsets.UTF_8);
        println("Decompiled batch of " + entries.size() + " functions");
    }
}
