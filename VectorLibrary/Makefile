all: fixed double

fixed: VectorFixed.c \
       VectorFixed.h \
       MatrixFixed.c \
       MatrixFixed.h \
       QuaternionFixed.c \
       QuaternionFixed.h

double: VectorDouble.c \
        VectorDouble.h \
        MatrixDouble.c \
        MatrixDouble.h \
        QuaternionDouble.c \
        QuaternionDouble.h

VectorFixed.c: Vector.c ConvertToFixed.pl
	perl ConvertToFixed.pl <$< >$@

VectorFixed.h: Vector.h ConvertToFixed.pl
	perl ConvertToFixed.pl <$< >$@

MatrixFixed.c: Matrix.c ConvertToFixed.pl
	perl ConvertToFixed.pl <$< >$@

MatrixFixed.h: Matrix.h ConvertToFixed.pl
	perl ConvertToFixed.pl <$< >$@

QuaternionFixed.c: Quaternion.c ConvertToFixed.pl
	perl ConvertToFixed.pl <$< >$@

QuaternionFixed.h: Quaternion.h ConvertToFixed.pl
	perl ConvertToFixed.pl <$< >$@

VectorDouble.c: Vector.c ConvertToDouble.pl
	perl ConvertToDouble.pl <$< >$@

VectorDouble.h: Vector.h ConvertToDouble.pl
	perl ConvertToDouble.pl <$< >$@

MatrixDouble.c: Matrix.c ConvertToDouble.pl
	perl ConvertToDouble.pl <$< >$@

MatrixDouble.h: Matrix.h ConvertToDouble.pl
	perl ConvertToDouble.pl <$< >$@

QuaternionDouble.c: Quaternion.c ConvertToDouble.pl
	perl ConvertToDouble.pl <$< >$@

QuaternionDouble.h: Quaternion.h ConvertToDouble.pl
	perl ConvertToDouble.pl <$< >$@
