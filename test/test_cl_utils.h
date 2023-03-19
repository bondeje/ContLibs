// MUST RUN WITH NDEBUG defined in order to pass

#include "../src/cl_core.h" // core includes cl_utils.h
#include <stdio.h>
#include <stdlib.h> // size_t, SIZE_MAX for size_t
#include <time.h>
#include <assert.h>

//#define INCLUDE_TEST_DEBUG // comment out to bypass crashing on success in test_debug()

int test_swaps() {
	printf("Testing cl_swap features...");
	void * buf;
	int isrc, idest;
	float fsrc, fdest;
	double dsrc, ddest;
	char csrc, cdest;

	int intvals[] = {1, 2, 3, 4, 5};
	// test cl_swaps4
	buf = malloc(sizeof(int));
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			isrc = intvals[i];
			idest = intvals[j];
			cl_swap(&intvals[i], &intvals[j], sizeof(int), buf);
			ASSERT((intvals[i] == idest) && (intvals[j] == isrc), "failed to swap index pair (%d, %d) from (%d, %d) != (%d, %d)\n", i, j, intvals[i], intvals[j], idest, isrc);
		}
	}
	free(buf);

	float fltvals[] = {1.2, 2.3, 3.4, 4.5, 5.6};
	buf = malloc(sizeof(float));
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			fsrc = fltvals[i];
			fdest = fltvals[j];
			cl_swap(&fltvals[i], &fltvals[j], sizeof(float), buf);
			ASSERT((fltvals[i] == fdest) && (fltvals[j] == fsrc), "failed to swap index pair (%d, %d) from (%f, %f) != (%f, %f)\n", i, j, fltvals[i], fltvals[j], fdest, fsrc);
		}
	}
	free(buf);

	double dblvals[] = {1.2, 2.3, 3.4, 4.5, 5.6};
	buf = malloc(sizeof(double));
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			dsrc = dblvals[i];
			ddest = dblvals[j];
			cl_swap(&dblvals[i], &dblvals[j], sizeof(double), buf);
			ASSERT((dblvals[i] == ddest) && (dblvals[j] == dsrc), "failed to swap index pair (%d, %d) from (%lf, %lf) != (%lf, %lf)\n", i, j, dblvals[i], dblvals[j], ddest, dsrc);
		}
	}
	free(buf);

	char charvals[] = {'a', 'b', 'c', 'd', 'e'};
	buf = malloc(sizeof(char));
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			csrc = charvals[i];
			cdest = charvals[j];
			cl_swap(&charvals[i], &charvals[j], sizeof(char), buf);
			ASSERT((charvals[i] == cdest) && (charvals[j] == csrc), "failed to swap index pair (%d, %d) from (%c, %c) != (%c, %c)\n", i, j, charvals[i], charvals[j], cdest, csrc);
		}
	}
	free(buf);

	// test cl_swaps3
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			isrc = intvals[i];
			idest = intvals[j];
			cl_swap(&intvals[i], &intvals[j], sizeof(int));
			ASSERT((intvals[i] == idest) && (intvals[j] == isrc), "failed to swap index pair (%d, %d) from (%d, %d) != (%d, %d)\n", i, j, intvals[i], intvals[j], idest, isrc);
		}
	}

	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			fsrc = fltvals[i];
			fdest = fltvals[j];
			cl_swap(&fltvals[i], &fltvals[j], sizeof(float));
			ASSERT((fltvals[i] == fdest) && (fltvals[j] == fsrc), "failed to swap index pair (%d, %d) from (%f, %f) != (%f, %f)\n", i, j, fltvals[i], fltvals[j], fdest, fsrc);
		}
	}
	
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			dsrc = dblvals[i];
			ddest = dblvals[j];
			cl_swap(&dblvals[i], &dblvals[j], sizeof(double));
			ASSERT((dblvals[i] == ddest) && (dblvals[j] == dsrc), "failed to swap index pair (%d, %d) from (%lf, %lf) != (%lf, %lf)\n", i, j, dblvals[i], dblvals[j], ddest, dsrc);
		}
	}

	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			csrc = charvals[i];
			cdest = charvals[j];
			cl_swap(&charvals[i], &charvals[j], sizeof(char));
			ASSERT((charvals[i] == cdest) && (charvals[j] == csrc), "failed to swap index pair (%d, %d) from (%c, %c) != (%c, %c)\n", i, j, charvals[i], charvals[j], cdest, csrc);
		}
	}
	
	printf("...PASS\n");
	return CL_SUCCESS;
}

int test_debug() {
	printf("Testing debug features...");

#ifdef NDEBUG
	printf("debug is off...");
	DEBUG_EXPR({
		printf("debug expression works if nothing happens here\n");
	});
	
	DEBUG_PRINT(("debug print works if nothing happens here\n"));

	ASSERT(1, "assert works if nothing happens here\n");
	ASSERT(0, "assert works if nothing happens here\n");
#else
	printf("debug is on...\n");
	DEBUG_EXPR({
		printf("debug expression works with NDEBUG off\n");
	})
	
	DEBUG_PRINT(("debug print works with NDEBUG off\n"));

	ASSERT(1, "assert works if nothing happens here\n");
	ASSERT(0, "assert works with NDEBUG off if program terminates after this. value = %d\n", 0);
#endif // NDEBUG

	printf("...PASS\n");

	return CL_SUCCESS;
}

// TODO: test with cl_reverse_buffered
int test_reverse() {
	printf("Testing cl_reverse...");

	int intvals[] = {1, 2, 3, 4, 5};
	int intvals_rev[] = {5, 4, 3, 2, 1};
	cl_reverse(&intvals[0], &intvals[4], sizeof(int));
	for (int i = 0; i < 5; i++) {
		ASSERT(intvals[i] == intvals_rev[i], "failed to reverse at index (%d): (%d) != (%d)\n", i, intvals[i], intvals_rev[i]);
	}

	float fltvals[] = {1.2, 2.3, 3.4, 4.5, 5.6};
	float fltvals_rev[] = {5.6, 4.5, 3.4, 2.3, 1.2};
	cl_reverse(&fltvals[0], &fltvals[4], sizeof(float));
	for (int i = 0; i < 5; i++) {
		ASSERT(fltvals[i] == fltvals_rev[i], "failed to reverse at index (%d): (%f) != (%f)\n", i, fltvals[i], fltvals_rev[i]);
	}

	double dblvals[] = {1.2, 2.3, 3.4, 4.5, 5.6};
	double dblvals_rev[] = {5.6, 4.5, 3.4, 2.3, 1.2};
	cl_reverse(&dblvals[0], &dblvals[4], sizeof(double));
	for (int i = 0; i < 5; i++) {
		ASSERT(dblvals[i] == dblvals_rev[i], "failed to reverse at index (%d): (%lf) != (%lf)\n", i, dblvals[i], dblvals_rev[i]);
	}

	char charvals[] = {'a', 'b', 'c', 'd', 'e'};
	char charvals_rev[] = {'e', 'd', 'c', 'b', 'a'};
	cl_reverse(&charvals[0], &charvals[4], sizeof(char));
	for (int i = 0; i < 5; i++) {
		ASSERT(charvals[i] == charvals_rev[i], "failed to reverse at index (%d): (%c) != (%c)\n", i, charvals[i], charvals_rev[i]);
	}

	// TODO: add a test where invalid reverse start/end are passed
	
	printf("...PASS\n");
	return CL_SUCCESS;
}

/*
void test_bisect_left() {
	printf("Testing bisect_left...\n");
	
	size_t N = 1e4;
	
	int * iarr = malloc(N*sizeof(int));
	for (size_t i = 0; i < N; i++) {
		iarr[i] = randrange(-(int) N/4, N/4);
	}
	
	qsort((void *) iarr, N, sizeof(int), compare(int,int));
	
	int val = randrange(-(int) N/4, N/4);
	long long ind = bisect_left((void *) iarr, (void *) &val, 0, N-1, sizeof(int), compare(int,int));
	
	assert(ind >= 0 || !printf("bisect_left failure: detected input error\n"));
	
	DEBUG_EXPR({
		if (ind > 0 && ind < N-1) {
			printf("inserting %d at %llu, sequenece (%llu, %llu, %llu) = (%d, %d, %d)\n", val, ind, ind-1, ind, ind+1, iarr[ind-1], iarr[ind], iarr[ind+1]);
		} else if (!ind) {
			printf("inserting %d at %llu, sequenece (%llu, %llu) = (%d, %d)\n", val, ind, ind, ind+1, iarr[ind], iarr[ind+1]);
		} else if (ind == N-1) {
			printf("inserting %d at %llu, sequenece (%llu, %llu) = (%d, %d)\n", val, ind, ind-1, ind, iarr[ind-1], iarr[ind]);
		} else if (ind == N) {
			printf("inserting %d at %llu, sequenece (%llu) = (%d, %d)\n", val, ind, ind-1, iarr[ind-1]);
		}
	})

	if (ind) {
		assert(iarr[ind-1] < val || !printf("bisect_left failure: insert at (%lld) results in previous value (%d) >= (%d)\n", ind, iarr[ind-1], val));
	}
	if (ind < N) {
		assert(iarr[ind] >= val || !printf("bisect_left failure: insert at (%lld) results in successive value (%d) < (%d)\n", ind, iarr[ind], val));
	}
	
	free(iarr);
	
	float * farr = malloc(N*sizeof(float));
	for (size_t i = 0; i < N; i++) {
		farr[i] = (float) random();
	}
	
	qsort((void *) farr, N, sizeof(float), compare(float,float));
	
	float fval = random();
	long long indf = bisect_left((void *) farr, (void *) &fval, 0, N-1, sizeof(float), compare(float,float));
	
	assert(indf >= 0 || !printf("bisect_left failure: detected input error\n"));
	
	DEBUG_EXPR({
		if (indf > 0 && indf < N-1) {
			printf("inserting %f at %llu, sequenece (%llu, %llu, %llu) = (%f, %f, %f)\n", fval, indf, indf-1, indf, indf+1, farr[indf-1], farr[indf], farr[indf+1]);
		} else if (!indf) {
			printf("inserting %f at %llu, sequenece (%llu, %llu) = (%f, %f)\n", fval, indf, indf, indf+1, farr[indf], farr[indf+1]);
		} else if (indf == N-1) {
			printf("inserting %f at %llu, sequenece (%llu, %llu) = (%f, %f)\n", fval, indf, indf-1, indf, farr[indf-1], farr[indf]);
		} else if (indf == N) {
			printf("inserting %f at %llu, sequenece (%llu) = (%f, %f)\n", fval, indf, indf-1, farr[indf-1]);
		}
	})

	if (indf) {
		assert(farr[indf-1] < fval || !printf("bisect_left failure: insert at (%lld) results in previous value (%f) >= (%f)\n", indf, farr[indf-1], fval));
	}
	if (indf < N) {
		assert(farr[indf] >= fval || !printf("bisect_left failure: insert at (%lld) results in successive value (%f) < (%f)\n", indf, farr[indf], fval));
	}
	
	free(farr);
	
	double * darr = malloc(N*sizeof(double));
	for (size_t i = 0; i < N; i++) {
		darr[i] = random();
	}
	
	qsort((void *) darr, N, sizeof(double), compare(double,double));
	
	double dval = random();
	long long indd = bisect_left((void *) darr, (void *) &dval, 0, N-1, sizeof(double), compare(double,double));
	
	assert(indd >= 0 || !printf("bisect_left failure: detected input error\n"));
	
	DEBUG_EXPR({
		if (indd > 0 && indd < N-1) {
			printf("inserting %f at %llu, sequenece (%llu, %llu, %llu) = (%f, %f, %f)\n", dval, indd, indd-1, indd, indd+1, darr[indd-1], darr[indd], darr[indd+1]);
		} else if (!indd) {
			printf("inserting %f at %llu, sequenece (%llu, %llu) = (%f, %f)\n", dval, indd, indd, indd+1, darr[indd], darr[indd+1]);
		} else if (indd == N-1) {
			printf("inserting %f at %llu, sequenece (%llu, %llu) = (%f, %f)\n", dval, indd, indd-1, indd, darr[indd-1], darr[indd]);
		} else if (indd == N) {
			printf("inserting %f at %llu, sequenece (%llu) = (%f, %f)\n", dval, indd, indd-1, darr[indd-1]);
		}
	})

	if (indd) {
		assert(darr[indd-1] < dval || !printf("bisect_left failure: insert at (%lld) results in previous value (%f) >= (%f)\n", indd, darr[indd-1], dval));
	}
	if (indd < N) {
		assert(darr[indd] >= dval || !printf("bisect_left failure: insert at (%lld) results in successive value (%f) < (%f)\n", indd, darr[indd], dval));
	}
	
	free(darr);
	
	printf("...Pass\n");
	return;
}

void test_bisect_right() {
	printf("Testing bisect_right...\n");
	
	size_t N = 1e4;
	
	int * iarr = malloc(N*sizeof(int));
	for (size_t i = 0; i < N; i++) {
		iarr[i] = randrange(-(int) N/4, N/4);
	}
	
	qsort((void *) iarr, N, sizeof(int), compare(int,int));
	
	int val = randrange(-(int) N/4, N/4);
	long long ind = bisect_right((void *) iarr, (void *) &val, 0, N-1, sizeof(int), compare(int,int));
	
	assert(ind >= 0 || !printf("bisect_right failure: detected input error\n"));
	
	DEBUG_EXPR({
		if (ind > 0 && ind < N-1) {
			printf("inserting %d at %llu, sequenece (%llu, %llu, %llu) = (%d, %d, %d)\n", val, ind, ind-1, ind, ind+1, iarr[ind-1], iarr[ind], iarr[ind+1]);
		} else if (!ind) {
			printf("inserting %d at %llu, sequenece (%llu, %llu) = (%d, %d)\n", val, ind, ind, ind+1, iarr[ind], iarr[ind+1]);
		} else if (ind == N-1) {
			printf("inserting %d at %llu, sequenece (%llu, %llu) = (%d, %d)\n", val, ind, ind-1, ind, iarr[ind-1], iarr[ind]);
		} else if (ind == N) {
			printf("inserting %d at %llu, sequenece (%llu) = (%d, %d)\n", val, ind, ind-1, iarr[ind-1]);
		}
	})

	if (ind) {
		assert(iarr[ind-1] <= val || !printf("bisect_right failure: insert at (%lld) results in previous value (%d) >= (%d)\n", ind, iarr[ind-1], val));
	}
	if (ind < N) {
		assert(iarr[ind] > val || !printf("bisect_right failure: insert at (%lld) results in successive value (%d) < (%d)\n", ind, iarr[ind], val));
	}
	
	free(iarr);
	
	float * farr = malloc(N*sizeof(float));
	for (size_t i = 0; i < N; i++) {
		farr[i] = (float) random();
	}
	
	qsort((void *) farr, N, sizeof(float), compare(float,float));
	
	float fval = random();
	long long indf = bisect_right((void *) farr, (void *) &fval, 0, N-1, sizeof(float), compare(float,float));
	
	assert(indf >= 0 || !printf("bisect_right failure: detected input error\n"));
	
	DEBUG_EXPR({
		if (indf > 0 && indf < N-1) {
			printf("inserting %f at %llu, sequenece (%llu, %llu, %llu) = (%f, %f, %f)\n", fval, indf, indf-1, indf, indf+1, farr[indf-1], farr[indf], farr[indf+1]);
		} else if (!indf) {
			printf("inserting %f at %llu, sequenece (%llu, %llu) = (%f, %f)\n", fval, indf, indf, indf+1, farr[indf], farr[indf+1]);
		} else if (indf == N-1) {
			printf("inserting %f at %llu, sequenece (%llu, %llu) = (%f, %f)\n", fval, indf, indf-1, indf, farr[indf-1], farr[indf]);
		} else if (indf == N) {
			printf("inserting %f at %llu, sequenece (%llu) = (%f, %f)\n", fval, indf, indf-1, farr[indf-1]);
		}
	})

	if (indf) {
		assert(farr[indf-1] <= fval || !printf("bisect_right failure: insert at (%lld) results in previous value (%f) >= (%f)\n", indf, farr[indf-1], fval));
	}
	if (indf < N) {
		assert(farr[indf] > fval || !printf("bisect_right failure: insert at (%lld) results in successive value (%f) < (%f)\n", indf, farr[indf], fval));
	}
	
	free(farr);
	
	double * darr = malloc(N*sizeof(double));
	for (size_t i = 0; i < N; i++) {
		darr[i] = random();
	}
	
	qsort((void *) darr, N, sizeof(double), compare(double,double));
	
	double dval = random();
	long long indd = bisect_right((void *) darr, (void *) &dval, 0, N-1, sizeof(double), compare(double,double));
	
	assert(indd >= 0 || !printf("bisect_right failure: detected input error\n"));
	
	DEBUG_EXPR({
		if (indd > 0 && indd < N-1) {
			printf("inserting %f at %llu, sequenece (%llu, %llu, %llu) = (%f, %f, %f)\n", dval, indd, indd-1, indd, indd+1, darr[indd-1], darr[indd], darr[indd+1]);
		} else if (!indd) {
			printf("inserting %f at %llu, sequenece (%llu, %llu) = (%f, %f)\n", dval, indd, indd, indd+1, darr[indd], darr[indd+1]);
		} else if (indd == N-1) {
			printf("inserting %f at %llu, sequenece (%llu, %llu) = (%f, %f)\n", dval, indd, indd-1, indd, darr[indd-1], darr[indd]);
		} else if (indd == N) {
			printf("inserting %f at %llu, sequenece (%llu) = (%f, %f)\n", dval, indd, indd-1, darr[indd-1]);
		}
	})
	
	if (indd) {
		assert(darr[indd-1] <= dval || !printf("bisect_right failure: insert at (%lld) results in previous value (%f) >= (%f)\n", indd, darr[indd-1], dval));
	}
	if (indd < N) {
		assert(darr[indd] > dval || !printf("bisect_right failure: insert at (%lld) results in successive value (%f) < (%f)\n", indd, darr[indd], dval));
	}
	
	free(darr);
	
	printf("...Pass\n");
	return;
}
*/