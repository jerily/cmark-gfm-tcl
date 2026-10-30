package require cmark_gfm 0.1.0

puts -nonewline [::cmark_gfm::render_gfm_html {# Title

| Language | Interface |
| --- | --- |
| Tcl | Commands |
| C/C++ | Native API |
}]
puts [::cmark_gfm::normalize_commonmark "  Title\n  =====\n"]
