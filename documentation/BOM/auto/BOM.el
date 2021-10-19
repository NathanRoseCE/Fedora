(TeX-add-style-hook
 "BOM"
 (lambda ()
   (TeX-add-to-alist 'LaTeX-provided-class-options
                     '(("article" "11pt")))
   (TeX-run-style-hooks
    "latex2e"
    "article"
    "art11"
    "fp"
    "booktabs"
    "ragged2e"
    "longtable"
    "hyperref"
    "geometry")
   (TeX-add-symbols
    '("product" 5)
    "totalttc"
    "inc"
    "TotalHT")
   (LaTeX-add-counters
    "cnt"))
 :latex)

