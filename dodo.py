import os

from doit.reporter import ConsoleReporter

class MyReporter(ConsoleReporter):
    def execute_task(self, task):
        self.outstream.write('%s:\n' % task.name)
        for action in task.actions:
            self.outstream.write('\t%s\n' % action)

DOIT_CONFIG = {'reporter': MyReporter,
               'verbosity': 2}

buildDir = "build/"
workDir = "../work/"
manageDir = "managementshim/"
libDir = "lib/"
benchmarkRootDir = "benchmarks/"
benchmarkDirs = [x[0]+"/" for x in os.walk(benchmarkRootDir)] #Get all the direct subdirectories of the benchmarks folder.
benchmarkDirs = benchmarkDirs[1:] #Exclude root dir

shim = manageDir+"management"
instr = libDir+"instructions"
managementInit = manageDir+"managementinit"
enclaveInit = libDir+"enclaveinit"

binaries = [buildDir+shim]
inits = [managementInit, enclaveInit]

riscvPrefix = "riscv64-unknown-linux-gnu-"
compiler = riscvPrefix+"gcc "
includeCompilerFlags = "-I"+libDir+" -I"+manageDir+ " "
bareCompileFlags = includeCompilerFlags + "-nostdlib -mcmodel=medany "
enclaveCompilerFlags = bareCompileFlags + "-T"+libDir+"enclavelink.ld "
shimCompilerFlags = bareCompileFlags + "-T"+manageDir+"managementlink.ld -Os"

allObjects = [shim, instr, libDir+"praesidioenclave", libDir+"praesidiouser", libDir+"praesidio"]
userExecs = []
compileFlagOutDeps = []
compileFlagOutDeps.append( (shimCompilerFlags, buildDir+shim+".out", [buildDir+val+".o" for val in [shim, managementInit, instr]]) )
for dir in benchmarkDirs:
    outputFile = dir+"enclave"
    userFile = dir+"user"
    userExecs.append(buildDir+userFile)
    binaries.append(buildDir+outputFile)
    allObjects.append(outputFile)
    allObjects.append(userFile)
    compileFlagOutDeps.append( (enclaveCompilerFlags, buildDir+outputFile+".out", [buildDir+val+".o" for val in [outputFile, enclaveInit, instr, libDir+"praesidioenclave", libDir+"praesidio"]]) )
    compileFlagOutDeps.append( ("", buildDir+userFile+".out", [buildDir+val+".o" for val in [userFile, instr, libDir+"praesidiouser", libDir+"praesidio"]]) )

def task_compile():
    for flags,out,deps in compileFlagOutDeps:
        yield {
            'name':     out,
            'actions':  [compiler + flags + " -o " + out + " " + (" ".join(deps))],
            'file_dep': deps,
            'targets':  [out],
            'clean':    True
        }

tmpAllDirs = [manageDir, libDir]
tmpAllDirs += benchmarkDirs
buildDirs = []
for val in tmpAllDirs:
    buildDirs.append(""+buildDir+val)

def task_mkdirs():
    for val in buildDirs:
        yield {
            'name':     val,
            'actions':  ["mkdir -p " + val],
            'file_dep': [],
            'targets':  [val],
        }

def task_init():
    for val in inits:
        yield {
            'name':     val+".o",
            'actions':  [compiler + " -c " + val+".s -o " + buildDir+val+".o"],
            'file_dep': [],
            'task_dep': ['mkdirs'],
            'targets':  [buildDir+val+".o"]
        }

def task_object():
    for obj in allObjects:
        yield {
            'name':     obj+".o",
            'actions':  [compiler + includeCompilerFlags + " -o " + buildDir+obj+".o -c "+obj+".c"],
            'file_dep': [buildDir+val+".o" for val in inits],
            'task_dep': ['mkdirs'],
            'targets':  [buildDir+obj+".o"],
            'clean':    True
        }

def task_bin():
    for val in binaries:
        yield {
            'name':     val+".bin",
            'actions':  [riscvPrefix+"objcopy -O binary " + val+".out " + val+".bin"],
            'file_dep': [val+".out"],
            'targets':  [val+".bin"],
            'clean':    True
        }

def task_copy():
    return {
        'actions':  ["cp " + buildDir+shim+".bin " + workDir+"riscv-isa-sim/"] + ["cp -r " + buildDir+benchmarkRootDir+" " + workDir+"buildroot_initramfs_sysroot/root/"] + ['echo "#" >> ../conf/linux_defconfig'] + ["touch "+buildDir+"copy.tmp"],
        'file_dep': [buildDir+shim+".bin"],
        'targets':  [buildDir+"copy.tmp"],
        'clean':    True
    }

# def task_dump():
#     return {
#         'actions': [riscvPrefix+"objdump -D " + output + " > " + dumpFile],
#         'file_dep': [output],
#         'targets': [dumpFile],
#         'clean': True
#     }
