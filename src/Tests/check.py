#!/usr/bin/python3
#
# usage:  python check.py <compiler> <program>
#         where
#           o you have first moved to the directory with the check program
#           o <compiler> is the full path name for the executable file
#           o <program> is a program name *without* extension
#
# example:
#
# >>> cd /home/IMADA/courses/cc/
# >>> python check.py SomeCompiler IfThen
#
# Checking: IfThen
# Compiling....Done
# Assembling...Done
# Executing....Done
# Correct output
# >>>

from sys import argv, stdout
import os
import time
import getopt

# ProgAndSolDir = "/home/IMADA/courses/cc/"
ProgAndSolDir = ""

def fileempty(filename):
   file = open(filename, "r")
   text = file.read()
   file.close()
   return len(text) == 0
   
#
# get maximum number of open files
#
MAXFD = os.sysconf('SC_OPEN_MAX')

#
# run a program until completion or timeout.
#
def execvp_wait(path, args, timeout):
    global TimedOut
    # fork child, closing all file descriptors
    pid = os.fork()
    if pid==0: # child
        # close all open file descriptors except
        # stdin,stdout & stderr
        for fd in range(3,MAXFD):
            try: os.close(fd)
            except os.error: pass

        # TODO: connect stdin,stdout & stderr to
        # something reasonable
        # exec the child
        try:
            os.execvp("./" + path,args)
        except Exception:
            # print traceback if exception occurs
            import traceback
            traceback.print_exc(file=os.sys.stderr)
        # always exit
        os._exit(1)
    else: # parent

        t = timeout
        while t>=0:
            # wait for completion
            child_pid,status = os.waitpid(pid, os.WNOHANG)
            if child_pid==pid:
                if os.WIFSTOPPED(status) or \
                   os.WIFSIGNALED(status):
                    return None
                elif os.WIFEXITED(status):
                    return os.WEXITSTATUS(status)
            # wait for a second
            time.sleep(1)
            t -= 1

        # not completed within timeout seconds
        TimedOut = 1
        os.kill(pid, 9)
        os.kill(pid+1, 9)

#
# Test
#
if __name__=="__main__":
    TimedOut = 0
    compiler = argv[1]
    program = argv[2]
    user = str(os.getuid())
    stdout.write("\n")
    stdout.write("Checking: " + program + "\n")
    stdout.write("Compiling....")
    stdout.flush()
    code = execvp_wait("checkwrap",
                       ["checkwrap", compiler,
                        ProgAndSolDir + program + ".src", user], 60)
    if TimedOut:
        stdout.write("compiler TIMED OUT")
    else:
        if code != 0:
            stdout.write("[compiler ERROR code " + str(code) + "]")
        else:
           stdout.write("Done")
    stdout.write("\n")
    
    if not TimedOut and code == 0:
        stdout.write("Assembling...")
        stdout.flush()
        code = execvp_wait("gccwrap", ["gccwrap", user], 60)
        stdout.write("Done")
        if code != 0:
            stdout.write(" [assembler ERROR code " + str(code) + "]")
        stdout.write("\n")

        TimedOut = 0
        stdout.write("Executing....")
        stdout.flush()
        if program != "Knapsack":
            code = execvp_wait("execwrap", ["execwrap", user], 60)
        else:
            code = execvp_wait("execwrap", ["execwrap", user], 10*60)
        if TimedOut:
            stdout.write("runtime TIMED OUT")
        else:
            if code == 0:
                stdout.write("Done")
                os.system("diff /tmp/" + user + "-output " + \
                          ProgAndSolDir + program + ".out > /tmp/" + \
                          user + "-difference")
                if not fileempty("/tmp/" + user + "-difference"):
                   stdout.write("\nINCORRECT output")
                else:
                   stdout.write("\nCorrect output")
            else:
                stdout.write("[runtime ERROR code " + str(code) + "]")
        stdout.write("\n")
