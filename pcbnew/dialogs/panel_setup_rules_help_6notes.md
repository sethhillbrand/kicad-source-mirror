### Notes

Version clause must be the first clause.  It indicates the syntax version of the file so that
future rules parsers can perform automatic updates.  It should be
set to "1".

Rules should be ordered by specificity.  Later rules take
precedence over earlier rules; once a matching rule is found
no further rules will be checked.

Use Ctrl+/ to comment or uncomment line(s).

`signal_length` vs `length`:

When both `signal_length` and `length` constraints apply to a routed item that belongs to a signal (i.e. the net has a non-empty signal name), the `signal_length` constraint is evaluated on the aggregate of all nets in that signal and takes precedence for violation reporting and length/delay tuning targets. The per-net `length` constraint continues to apply to nets that are not part of any multi‑net signal group.

Time-domain targets:

Both `length` and `signal_length` support `(time_domain yes)` for propagation delay based constraints. For `signal_length` with time domain enabled, the specified delay window applies to the total signal delay; the router/tuner subtracts the already-routed delay contribution of the other nets in the signal to derive the remaining per‑net tuning requirement.


<br><br><br>