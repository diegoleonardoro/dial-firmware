---
description: Pick up where the last session left off and give Diego the next step
---

Start of a new session. Work out where the project stands and give the next step. Read-only.

1. Read `~/dial-app/docs/STATUS.md`, starting with "Where we left off". Then read the current
   phase in `~/dial-app/docs/PLAN.md` and the last entry of `~/dial-app/docs/LOG.md`.
2. Run `git status --short` and `git log --oneline -3` in `~/dial-firmware` and `~/dial-app`.
   Mention any uncommitted or unpushed work.
3. Open the file the "Next" line points to (for example `~/dial-firmware/BREADBOARD.md` or a
   part of `~/dial-firmware/README.md`) and read that step.
4. Reply with:
   - one or two lines: where we are (phase, step, what was last done);
   - the next step written out in full: numbered actions, exact holes/pins/menus, what Diego
     should see, and what to report back. Only the next step, not the rest of the plan;
   - any open decision from STATUS that blocks this step (otherwise leave it out).

Don't change any files. When Diego reports the step done, tick it in `PLAN.md` and update
`STATUS.md` ("Where we left off") and `LOG.md`.
