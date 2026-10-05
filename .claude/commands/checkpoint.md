---
description: Update PLAN, STATUS, DECISIONS and LOG from this session's work, then suggest commits
argument-hint: "[optional notes, e.g. results Diego measured on the hardware]"
---

Bring the tracking files in `~/dial-app/docs/` up to date with what happened this session.
Extra notes from Diego: $ARGUMENTS

1. Work out what changed: this conversation, plus `git status` and `git log` since the last
   `LOG.md` entry in both `~/dial-firmware` and `~/dial-app`.
2. `PLAN.md`: tick the boxes whose evidence now exists (file, commit, or a result Diego reported,
   with the number where the step asks for one). Add new steps that came up, tagged **ADDED**.
   Don't tick anything on assumption; if unsure, leave it unticked and list it in your reply.
3. `STATUS.md`: rewrite the snapshot (date, phase, versions, done, in progress, blocked,
   decisions needed, next 3 actions). Keep it under 40 lines.
4. `DECISIONS.md`: append one entry per design choice made this session (date, decision, why,
   where it lives). Never rewrite old entries; a reversal is a new entry.
5. `LOG.md`: append one dated entry (repos touched, what changed, commit hashes). Never edit
   older entries.
6. If `PROTOCOL.md` or `SPEC.md` changed in either repo, diff the two copies and report any
   difference.
7. Show a short summary of the edits, then suggest commit messages: one for `dial-app` (always,
   for `docs/`) and one for `dial-firmware` if it has changes. Don't commit unless Diego says so.
