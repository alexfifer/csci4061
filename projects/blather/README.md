# CSci 4061 Project 2: Blather Chat Server/Client

## Group Members

Adam Peterson : pet03153
Alex Fifer : fifer011

## Important Notes

Our most troublesome error was one where Valgrind reported a memory leak having to do with our
`pthread_create()` calls in `bl_client.c`. This was solved by opening both `to_client` and
`to_server` FIFOs with `O_RDWR` to prevent blocking behavior, rather than using either `O_RDONLY`
or `O_WRONLY`.

At the time of submission we pass all tests without issue.

For the majority of the project we followed the guide posted on the course website. All of the code
is original to this exercise with the exeception of what was provided through the course, to include
both `simpio` files, the code from `simpio_demo.c` that was used in `bl_client.c`, and the
`test_Makefile` suite.
