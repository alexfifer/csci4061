# CSci 4061 Project 1: Going Commando

### Overall structure of a shell:
```text
repeat forever
  read one line
  parse the command into a list of arguments
  if the line starts with a command name (e.g. cd and exit)
  then
    perform the function (if it's exit, break out of the loop)
  else (it invokes a program, e.g. ls and cat)
    execute the program
```

### Basics of operations:
- use `fork()` to clone a process
- use `execvp()` to run a new program in the background/foreground
- use `dup()` and `dup2()` to redirect standard output
- use `pipe()` to create pipes

### Things to Note:
- The child processes (jobs) that `commando` starts do not show any output by default and run
  concurrently with the main process which gives back the `@>` prompt immediately. This is different
  from a normal shell such as `bash` which starts jobs in the foreground, shows their output
  immediately, and will wait until a job finishes before showing the command prompt for additional
  input.
- The output for all jobs is saved by `commando` and can be recalled at any time using the
  `output-for int` built-in command.
- Not all of the built-in commands are shown in the demo but each will be discussed in later sections.
- It should be clear that `commando` is not a full shell (no signals, built-in scripting, or pipes),
  but it is a mid-sized project which will take some organization. Luckily, this document prescribes
  a simple architecture to make the coding manageable.


