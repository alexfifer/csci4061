# CSci 4061 Project 1: Going Commando

## Group Members

Adam Peterson : pet03153
Alex Fifer : fifer011

## Important Notes

When working on the project we noticed the contents of `test-data/` would sometimes become modified and
the size of `stuff/` would change. This affected our test output and would throw errors until we replaced
the directory with its original version. We are not sure why this occurred.

At the time of this submission we pass all of the tests with flying colors, on our own machines. We
noticed that we would fail the stress tests 19 and 20 on apollo. We are also unsure about why this
happened. When submitting to Gradescope initially test 17 for `pause` failed. We have since modified
the function call to `pause-for()` to be passed both a `long` and `int` exactly in hopes of clearing
this error, whereas before we passed two `int`. Since this passed on both our systems and apollo
this error is questionable but does not concern us.

For the majority of the project we followed the guide posted on the course website. All of the code
is original to this exercise with the exeception of what was provided through the course, to include
the `parse_into_tokens()` function.
