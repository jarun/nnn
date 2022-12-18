# Contributing
Contributions to nnn are welcome! There's always an open issue with the current ToDo list, which contains the proposed features for the next release you can get your hands on. Any small changes or ideas should go in there, rather than in a separate issue.

Before suggesting changes, please read a bit about [the design principles nnn follows](https://github.com/jarun/nnn/wiki/concepts#design), and make sure you aren't breaking any of them.

Feel free to join the [Discussions](https://github.com/jarun/nnn/discussions). We highly recommended a discussion before raising a PR.

## Coding standard

`nnn` follows the Linux kernel coding style closely. The C source code uses TABs and the plugins use 4 spaces for indentation.

- Code changes should not break the patch framework. Please run `make checkpatches` to ensure.
- Run `make shellcheck` if adding/modifying plugins.

CI runs patch framework sanity test and `shellcheck`. Please watch out for any failures after raising the PR.

## Resources
The [wiki](https://github.com/jarun/nnn/wiki/Developer-guides) has some resources for developers you might be interested in: building, debugging...

## Communication
* [Gitter chat](https://gitter.im/jarun/nnn)
* [GitHub team](https://github.com/nnn-devs) (if you plan on contributing regularly, ask for an invitation).
