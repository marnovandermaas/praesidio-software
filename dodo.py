
main = "management"
instr = "instructions"
init = "init"

depDict = {
    main: [instr],
    instr: [main],
}

output = "run.out"
binaryFile = "management.bin"
dumpFile = "memory.dump"

riscvPrefix = "riscv64-unknown-linux-gnu-"
compiler = riscvPrefix+"gcc"
compilerFlags = " -nostdlib -mcmodel=medany -Tlink.ld "

def task_compile():
    return {
        'actions': [compiler + compilerFlags + " -o " + output + " " + init + ".o " + (' '.join([key + ".o" for key in depDict.keys()]))],
        'file_dep': [init+".o"] + [key+".o" for key in depDict.keys()],
        'targets': [output],
        'clean': True
    }

def task_init():
  return {
    'actions': [compiler + " -c " + init + ".s -o " + init + ".o"],
    'file_dep': [],
    'targets': [init + ".o"]
  }

def task_object():
    for key in depDict.keys():
        yield {
            'name': key,
            'actions': [compiler + compilerFlags + " -c %s" % (key + ".c")],
            'file_dep': [init+".o"] + [key + ".c", key + ".h"] + [val + ".h" for val in depDict[key]],
            'targets': [key + ".o"],
            'clean': True
        }

def task_bin():
  return {
    'actions': [riscvPrefix+"objcopy -O binary " + output + " " + binaryFile,
        'cp ' + binaryFile + " ../../work/riscv-isa-sim/"],
    'file_dep': [output],
    'targets': [binaryFile],
    'clean': True
  }

def task_dump():
    return {
        'actions': [riscvPrefix+"objdump -D " + output + " > " + dumpFile],
        'file_dep': [output],
        'targets': [dumpFile],
        'clean': True
    }
