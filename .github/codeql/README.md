# Security analysis

The CodeQL workflow runs the `security-extended` query suite for the SDK language and GitHub Actions on pull requests, pushes, merge queues and a weekly schedule. It can also be started manually.

The SDK scan traces a clean Clang build of the SDK, OpenTelemetry plugins and examples, with tests and compiler caching disabled. Compiled dependencies and generated headers remain part of extraction.

Results are published to **Security and quality → Code scanning**, with a separate category for each language. Review initial findings and record a reason for every dismissed alert.

After merging, configure a branch ruleset with **Require code scanning results → CodeQL → Security alerts: High or higher** for supported branches. Successful workflow execution alone does not enforce this threshold. Track the latest analysis commit/date, extraction errors and open findings by security severity.
