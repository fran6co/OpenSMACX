// Decompile one function at a specified address to a text file.
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

public class DecompileFunction extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] arguments = getScriptArgs();
        if (arguments.length != 2) {
            throw new IllegalArgumentException("expected function address and output path");
        }

        Address address = toAddr(arguments[0]);
        Function function = getFunctionAt(address);
        if (function == null) {
            throw new IllegalArgumentException("function entry not found at " + address);
        }

        DecompInterface decompiler = new DecompInterface();
        try {
            if (!decompiler.openProgram(currentProgram)) {
                throw new IllegalStateException(decompiler.getLastMessage());
            }
            DecompileResults results = decompiler.decompileFunction(function, 120, monitor);
            if (!results.decompileCompleted()) {
                throw new IllegalStateException(results.getErrorMessage());
            }
            Path output = Paths.get(arguments[1]);
            Path parent = output.toAbsolutePath().getParent();
            if (parent != null) {
                Files.createDirectories(parent);
            }
            Files.writeString(
                output,
                results.getDecompiledFunction().getC(),
                StandardCharsets.UTF_8);
            println("Decompiled " + function.getName(true) + " to " + output);
        }
        finally {
            decompiler.dispose();
        }
    }
}
