// Export references that target the interior of a recognized function.
// @category OpenSMACX

import ghidra.app.script.GhidraScript;
import ghidra.framework.Application;
import ghidra.program.model.listing.CodeUnit;
import ghidra.program.model.listing.CodeUnitIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;

import java.io.BufferedWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class ExportInteriorReferences extends GhidraScript {
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
        int count = 0;
        try (BufferedWriter writer = Files.newBufferedWriter(
                output, StandardCharsets.UTF_8)) {
            writer.write(
                "program_sha256,ghidra_version,source,target,target_function,type,primary\n");
            CodeUnitIterator units = currentProgram.getListing().getCodeUnits(true);
            while (units.hasNext()) {
                CodeUnit unit = units.next();
                for (Reference reference : unit.getReferencesFrom()) {
                    if (!reference.getFromAddress().isMemoryAddress() ||
                            !reference.getToAddress().isMemoryAddress()) {
                        continue;
                    }
                    Function function = getFunctionContaining(reference.getToAddress());
                    if (function == null ||
                            function.getEntryPoint().equals(reference.getToAddress()) ||
                            function.getBody().contains(reference.getFromAddress())) {
                        continue;
                    }
                    writer.write(String.join(",",
                        executableHash,
                        ghidraVersion,
                        String.format("0x%08X", reference.getFromAddress().getOffset()),
                        String.format("0x%08X", reference.getToAddress().getOffset()),
                        String.format("0x%08X", function.getEntryPoint().getOffset()),
                        reference.getReferenceType().toString(),
                        Boolean.toString(reference.isPrimary())));
                    writer.write("\n");
                    count++;
                }
            }
        }
        println("Exported " + count + " interior function references to " + output);
    }
}
