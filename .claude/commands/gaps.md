---
description: Compare the plan against both repos and list missing, unproven or unplanned work
---

Audit `~/dial-app/docs/PLAN.md` against the real repos `~/dial-firmware` and `~/dial-app`.
Read-only: report, don't fix. Read `SPEC.md`, `PROTOCOL.md`, both `CLAUDE.md` files, and the
code the plan refers to.

Report four lists, each item with file paths or line numbers as evidence:

1. **Ticked without evidence:** boxes marked done where the file, commit or recorded result
   isn't there.
2. **Done but not in the plan:** work in the repos that PLAN.md doesn't mention.
3. **Spec vs code mismatches:** things `SPEC.md` or `PROTOCOL.md` define that the firmware or app
   doesn't implement, or implements differently. Check that both `PROTOCOL.md` copies are
   identical and that UUIDs and event codes match `dial_config.h`, `protocol.h` and the Swift
   constants (once they exist).
4. **Missing steps** that a project like this usually needs and the plan never mentions, for
   example: tests, error handling (BLE disconnects, unsupported exposure combinations),
   firmware update path once the enclosure is closed, backups and tags, documentation,
   privacy/App Review items. Only list ones that apply to the current or next phase, plus at
   most 3 later ones.

End with: the top 3 gaps to act on first, and proposed PLAN.md lines for them (tagged
**ADDED**) that Diego can approve before you add them.
