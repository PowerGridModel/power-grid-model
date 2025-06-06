<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Advanced terminology

The power grid model is a large project with many assumptions, as well as design and implementation choices.
It combines deep electrical engineering knowledge and modeling with physics, mathematics and programming.
All those fields of expertises also come with their own terminology.
In an attempt to ease communication between experts from different fields, this section contains a (non-exhaustive)
glossary of the terminology used within the power grid model knowledge base.

```{contents}
```

## Programming terminology

### Bug

A bug is a problem with the code, in the sense that the code behaves differently from the intended behavior.
Examples are crashes, incorrectly raising exceptions and incorrect results.
We take bugs very seriously, and will try to fix them as soon as possible.
If you find a bug, or expect to have found one, please report it.
We will respond as soon as possible.

Depending on the nature and impact of the bug, we will post announcements via our
[mailing list](https://lists.lfenergy.org/g/powergridmodel).

#### Security vulnerability

These bugs are the most severe type of bugs, as they may give attackers access to data, secrets or controls they would
otherwise not have access to.
Due to the nature of the power grid model as a library, it does not directly interface with the outside world, like a
REST API can do.
Only attackers that already have illegitimate access to the data or system can affect the results, but no more than they
would otherwise have without the use of the library.

That said, the power grid model still contains a number of critical points that deserve special attention of the user of
the power grid model - although no more than any other library:

* The C API is a native data interface. Although the power grid model can check whether the input contain null-pointers
  (`nullptr`/`NULL`/`0` values where they are not allowed) and can act accordingly, it is not possible to check whether
  a non-null pointer was actually valid.
  The behavior of the power grid model in such cases is [undefined](#undefined-behavior-ub-as-a-bug).
  This is also true for any other native interface that deals with pointers.
  It is the users' responsibility to provide correct input data.
  See also [below](#bad-input).
* If the input data provided to the power grid model is incorrect, the power grid model cannot guarantee that it will be
  able to find and handle that correctly.
  The behavior is [undefined](#undefined-behavior-ub-as-a-bug).
  This is also true for any other interface, including native data interfaces.
  [It is the users' responsibility](https://en.wikipedia.org/wiki/Garbage_in,_garbage_out) to provide correct input
  data.
  See also [below](#bad-input).

#### Incorrect results

These bugs are the second most severe kind, as they directly impact users without reporting them.
They may remain hidden until they resurface in a completely different location.
Encountering such bugs may result in wrong decisions in control logic or incorrect conclusions in scientific research.

We take these bugs very seriously.
If you encounter them, please report them.
We will always post announcements regarding bugs of this nature.

#### Nonsensical results

Sometimes, bugs in the code can cause clearly nonsensical results, like not-a-number (NaN/`np.nan`)
values in the output.
Like in the case of [incorrect results](#incorrect-results), no exceptions are raised (if there were, it would not be a
bug of this type) and therefore may be hidden for a long time until they first appear.
However, they are easy to detect, especially in production environments with (real-time) monitoring and other tools,
programs and libraries that do their own error handling.

Their visibility makes solving them usually easy.
If you encounter them, please report them.
We will usually post announcements regarding bugs of this nature.

#### Undefined behavior bugs

See also [below](#undefined-behavior-ub-as-a-bug).

Although the power grid model takes all the care it can in preventing
[undefined behavior](#undefined-behavior-ub-as-a-bug) like infinite loops or extreme blow-up of the data usage, it is
still possible that such behavior exists in the code.
If you find any such issues, please report the issue and we will fix them as soon as possible.

#### Incorrect exceptions

Sometimes, exceptions are raised when they should not be.
For example, a `NotObservableError` that is raised when the current scenario actually contains an observable grid, would
be a bug.
They can be a headache to users, because it is difficult to know whether the exception was caused by
[bad data](#bad-input) or by a power grid model bug, even for the power grid model maintainers.

If you find any such issues, please report the issue and explicitly mention that you suspect a bug in the power grid
model, together with a reasoning as to why you think so.
That way, the maintainers can take extra care to investigate before coming back to you with an answer.

#### Incorrect/outdated documentation

The power grid model maintainers will always try to keep the documentation up-to-date.
However, it is always possible that sections of the documentation become outdated.
Please help us keep our documentation up-to-date by reporting and/or fixing issues.

### Bad input

This is a problem with the input data.
They cannot be considered bugs in the power grid model and instead are the users' responsibility.

In certain cases, the [data validator](../user_manual/data-validator.md) can catch data issues.
However, it is not guaranteed to provide an exhaustive check.

#### Unsupported input

The power grid model explicitly forbids certain (combinations of) values.
For example, a negative `u_rated` on a node is not allowed.
Unsupported input will always be documented in the power grid model documentation.

Checking every single combination of input parameters would come at a significant performance cost.
In addition, for most use cases, incorrect input data happens mostly during the development phase of a user workflow.
In production environments, when performance is usually important, the data is usually already sanitized and can be
assumed to be correct.
Because of this, assuring the
[correctness of the input data is the users' responsibility](https://en.wikipedia.org/wiki/Garbage_in,_garbage_out).

However, most data errors of this kind can be found using the [data validator](../user_manual/data-validator.md).

#### Incorrect input

The power grid model assumes that the input data is correct.
Of course, the power grid model cannot know whether a user provided the grid correctly.
For example, if the user adds a source to the wrong node, the power grid model can only obey.

Input correctness is entirely the users' responsibility.
The [power-grid-model-ds visualiser](https://power-grid-model-ds.readthedocs.io/en/stable/visualizer.html) may provide
some tools to help with the investigation.

#### Unreasonable input

The power grid model assumes that the input data is correct and makes assumptions about typical use cases of the grid
based on electrical engineering expert knowledge.
For instance, while a line with an impedance of $10^{-50}\,\Omega$ is mathematically possible, it does not make sense
from an electrical engineering point of view.
Making the calculations mathematically stable under such extreme situations would come at an excessive temporal and
spatial performance cost.

The threshold of what can and cannot be considered "reasonable" is a vague gray area, potentially spanning multiple
orders of magnitude.
As a result, it is possible that the assumptions of the power grid model are currently too restrictive.
If you have a use case for which the power grid model currently does not provide the correct results as a consequence of
such optimizations, please consider [contributing](../user_manual/model-validation.md#test-case-creation) or reporting
the use case.
It may be possible to improve the behavior in such edge cases, like we did for
[sources with large `sk`](https://github.com/PowerGridModel/power-grid-model/issues/458).

Adding support for previously unsupported edge cases due to this type of assumptions is considered a new feature.

#### Experimental features

Experimental features are features that are still under development.
The behavior of the power grid model is [undefined](#undefined-behavior) when experimental features are used.
The data validator and/or documentation may not capture all the supported and unsupported values for the data.

We do not guarantee anything concerning experimental features and using and enabling them is entirely up to the user.
Any input regarding experimental features is always appreciated.

### Edge case

A scenario in which the behavior of the program changes significantly when one or more parameters change slightly.
Examples include the following.

* The input value `0` when taking the square root of a floating point:
  * A positive value will result in valid output data
  * A negative value will result in not-a-number values (NaN/`np.nan`); spurious
* Saddle-points in equations with many local optima may result in different optima, e.g.:
  * Different local optima (not spurious)
  * Short-circuit solutions (spurious)
  * Divergent domain (spurious)
  * Differences in amount of iterations before convergence is reached (not spurious)

#### Non-spurious edge cases

In non-spurious edge cases, the current behavior of the power grid model is actually correct.

Many edge cases are inherent to the problem statement are not necessarily bugs.
For instance, if it takes a large number of iterations before a solution is found in an iterative calculation method,
that is probably annoying and something that can be improved on, but not it is not necessarily directly a real issue
with the implementation.
Even if you encounter `IterationDiverge` errors, in many cases, increasing the error tolerance and/or maximum number of
iterations may fix your problem.

Some edge cases may be input data errors.
Please always double-check that the data is valid before reporting an issue, e.g., by running our
[data validator](../user_manual/data-validator.md).
Please also check whether your input data is correctly created and contains no obvious errors like sensors that are
marked as measuring a node while, in fact, they are actually measuring branch flow.

Other examples of non-spurious edge cases are cases in which the power grid model reports a `NotObservableError`.

#### Spurious edge cases

Some edge cases are actual problems with the code.
Some of them may be easy to solve, while others may be extremely difficult to find and/or resolve.
Occasionally, even, more research is still needed before a satisfactory fix to the edge case can be found.
An example of the latter is shown in [this paper](https://doi.org/10.48550/arXiv.2504.11650), which was published as
collaboration between ICT for industy and power grid model.

#### Resolving edge cases

If you encounter any edge case, please double-check your data quality, e.g., by running our
[data validator](../user_manual/data-validator.md) and verifying that the source of your data is
up-to-date and correct.
Please also try things like increasing the error tolerance or the number of iterations (when applicable) to see if that
solves your issue.
If it doesn't, the problem you found may be a bug in the power grid model.

If you encounter such edge cases and are sure that the current behavior is [incorrect](#spurious-edge-cases), please
consider [contributing your findings](../user_manual/model-validation.md#test-case-creation) to help us improve the
power grid model.

### Undefined behavior

A class of behavior of the code that is considered erroneous usage.
[cppreference.com](https://en.cppreference.com/w/cpp/language/ub.html) describes undefined behavior (UB) as follows.

> Renders the entire program meaningless if certain rules of the language are violated. [...]

This is opposed to observable behavior, which it describes as follows.

> The C++ standard precisely defines the observable behavior of every C++ program that does not fall
> into one of the [undefined behavior] classes.

#### Classes of UB

There are a number of classes of UB.
We try to follow the [conventions of the C++ standard](https://en.cppreference.com/w/cpp/language/ub.html) as much as
possible.
Here is a summary of the classes relevant to end-users, i.e, we omit things like syntax errors.

##### Implementation-defined

The behavior of the program may differ between versions of the power grid model, compilers, versions of the standard
library, platforms and architectures, usually intentionally.

Precision falls in this category, as differences between versions of the power grid model may introduce small deviations
(e.g., rounding or truncation) that propagate.

Other examples include the [native data interface](native-data-interface.md), which, by design, depends on the platform
and architecture.

Most effects on the results introduced by implementation-defined behavior are small.
However, in the case of [edge cases](#edge-case), a small difference may result in completely different outcomes.

Unfortunately, there is not much that the power grid model can do make the implementation-defined behavior more
consistent, except catching those edge cases that are easy to catch, like taking the square root of a negative number.

##### Unspecified behavior

The behavior of the program varies between implementations, and the conforming implementation is not required to
document the effects of each behavior.
Examples are the internal implementation, features that are still under development/experimental and the contents of the
output of failing scenarios if exceptions were thrown and `continue_on_batch_error=True`.

Encountering unspecified behavior is considered a problem with the user input.

Unspecified behavior shall **never** be considered a bug from a user point of view.
The developers may change the behavior at any time without notice.
For example, a new exception may be thrown that changes the behavior from unspecified behavior to specified
exception-throwing.

**NOTE:** PRs changing unspecified behavior may still be marked as a bugfix PR, if developers consider the previous
behavior unexpected and/or undesirable.
Examples are fixes to the behavior of a new feature that is still experimental/under development, but for which the
developers thought that the behavior was already correctly implemented, before they found out that it was not.

##### Undefined behavior (subclass)

Undefined behavior is also a particular subclass of the superset of all undefined behavior that is defined as follows:

> There are no restrictions on the behavior of the program.

Examples include if a C API user provides an invalid pointer to the power grid model C API.
While the C API can still compare and handle null-pointer (`nullptr`/`NULL`/`0`) values, it has no way of knowing what
will happen if the user provides a non-null pointer to the wrong memory location.

Encountering undefined behavior (subclass) may result in crashes (but not necessarily),
[security vulnerabilities](#undefined-behavior-bugs), or anything else.
It is a common misconception that the OS will take care of such edge cases by crashing fatally.
It is not required to do so and the behavior may even change case-by-case.

#### Undefined behavior (UB) as a feature

The power grid model attempts to define its API as future-proof as possible.
Designing it well helps in maintaining backwards compatibility, stability and an optimal user experience without being
in the way of adding new features.
That means that decisions regarding some edge-case behavior need to be postponed until it is needed to make a decision
on it.
Defining such cases as [undefined behavior](#undefined-behavior) enables this.
The following example illustrates this.

The power grid model uses enum values to set discrete options.
The list of supported enum values may be extended at a later stage (e.g., to support a new calculation type).
Consider the `enum A { a = 0, b = 1 };`.

* If the behavior is _defined_ to throw an error if any other value is supplied, then it will never be possible to
  extend the enum with new options, because doing so would be a breaking change.
* On the other hand, if the behavior is [_unspecified_](#unspecified-behavior) and the current behavior "just so happens
  to be" that it throws an error if another value is supplied, then it is still possible to at a later date without
  changing previously _defined_ behavior.

In the second case, the fact that the original behavior was [_unspecified_](#unspecified-behavior) enabled future
compatibility while still providing useful feedback to the user that an unsupported option was provided.

This example shows that undefined behavior is not necessarily bad - it just needs to be handled with care.
It does so by explicitly creating a boundary between what is the program's responsibility and what is the users'.

#### Undefined behavior (UB) as a bug

Triggering undefined behavior is a potential source for [security vulnerabilities](#undefined-behavior-bugs) and should
therefore be avoided.
Triggering undefined behavior should always be conidered a bug, regardless of whether it is caused by an incorrect
internal implementation of the power grid model or a result of incorrect usage by a user.

To help both users and maintainers to find incorrect usage of and bugs in the power grid model, it uses an error
handling mechanism.
It also has extended assertion checks in Debug build configuration used in test and development environments to prevent
internal bugs.

To prevent, find and fix harder-to-find instances of undefined behavior, the power grid model uses tools like
AddressSanitizer and SonarQube Cloud.
We strongly recommend all users to do the same - especially the ones using the C API.

If you find or expect any undefined behavior, please report it and/or
[contribute](../user_manual/model-validation.md#test-case-creation) with a reproducible case, as doing so helps
improving the quality and user experience of the power grid model.
