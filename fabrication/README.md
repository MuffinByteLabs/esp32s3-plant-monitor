Factory output files live here, one folder per ordered revision.

revA/ — ordered at JLCPCB 2026-08-21 (5 boards, Standard PCBA). Frozen: the
exact uploaded GERBER-*.zip (drill files inside), BOM-*.csv, and CPL-*.csv,
plus ORDER_NOTES.md (§7 = the as-ordered record and order-day lessons).
Note: final part rotations were confirmed/corrected in JLC's placement preview
after CPL upload — the order page's production files are the rotation truth.
Still to add: the order-confirmation PDF, and the ordering commit hash.

For future revisions, same recipe: freeze the exact uploaded zip + BOM + CPL,
the order confirmation, and a note of the git commit/tag that produced them.
