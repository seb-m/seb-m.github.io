clean:
	find . \( -name '*~' -or \
		-name '#*' -or \
		-name '*.pyc' -or \
		-name '.DS_Store' -or \
		-name '*.pyo' \) \
		-print -exec rm {} \;
