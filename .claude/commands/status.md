---
description: Report what's done, in progress, blocked and next, from the plan and both repos
---

Report the project status. Read-only: do not edit any file.

1. Read `~/dial-app/docs/STATUS.md`, `~/dial-app/docs/PLAN.md`, and the last 3 entries of
   `~/dial-app/docs/LOG.md`.
2. Look at both repos (`~/dial-firmware`, `~/dial-app`): `git status -sb`, `git log --oneline -5`,
   `git tag`. Note uncommitted or unpushed work.
3. Reply in this shape, short:
   - **Phase:** current phase and how many of its "Done when" boxes are ticked (x of y).
   - **Done since last checkpoint:** commits or file changes newer than the last LOG entry.
   - **In progress / blocked:** with the reason.
   - **Decisions needed:** from STATUS.md.
   - **Next 3 actions.**
   - **Drift:** anything where STATUS.md or PLAN.md disagrees with the repos (say "none" if none).
