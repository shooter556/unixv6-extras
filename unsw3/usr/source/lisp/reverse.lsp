(de reverse (list)
	((label reverse1
		(lambda (list m)
			(cond ((atom list) m)
				(t (reverse1
					(cdr list)
					(cons (car list) m)))
			)
		)
	) list nil)
)
