;; This buffer is for text that is not saved, and for Lisp evaluation.
;; To create a file, visit it with C-x C-f and enter text in its buffer.

(defun pov-online-search nil
  "Search the POV-Ray site and on-line documentation for a keyword"
  (interactive)
  (let* ((default (current-word))
	 (input (completing-read
		 (format "lookup keyword (default %s): " default)
		 pov-keyword-completion-alist))
	 (kw (if (equal input "")
		 default
	       input)))
    (browse-url (concat "http://www.google.com/custom?q=" 
			(browse-url-url-encode-chars kw "[#?&/]")
			"&sa=Google+Search&domains=povray.org%3B+news.povray.org%3B+"
			"www.povray.org&sitesearch=www.povray.org"))))
