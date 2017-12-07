#include "Ht.h"
using namespace Ht;

void usage(char *);

int main(int argc, char **argv)
{
	uint64_t len;

	// check command line args
	if (argc == 1) {
		len = 100;  // default len
	} else if (argc == 2) {
		len = atoi(argv[1]);
		if (len <= 0) {
			usage(argv[0]);
			return 0;
		}
	} else {
		usage(argv[0]);
		return 0;
	}

	printf("Running with len = %llu\n", (long long)len);
	fflush(stdout);

	CHtHif *pHtHif = new CHtHif();
	CHtAuUnit *pAuUnit = new CHtAuUnit(pHtHif);

	// Send arguments to all units using messages
	pHtHif->SendAllHostMsg(LEN, (uint64_t)len);

	// Send calls to unit
	pAuUnit->SendCall_htmain();

	// Artificially wait until some time has passed (and reset deasserts)
	usleep(100000);

	int errCnt = 0;

	uint64_t wrData = 0x5a5a5a5a5a;
	pHtHif->UserIOCsrWr(8, wrData);
	uint64_t rdData = pHtHif->UserIOCsrRd(8);

	if (rdData != wrData) {
		printf("ERROR: CSR Read data did not match Write data!\n");
		errCnt++;
	}

	// Wait for return
	uint64_t error[8];
	while (!pAuUnit->RecvReturn_htmain(error[0], error[1], error[2], error[3], error[4], error[5], error[6], error[7]))
		usleep(1000);

	for (int i = 0; i < 8; i++) {
		if (error[i] != 0) {
			printf("ERROR: Found %ld mismatches on lane %d!\n", error[i], i);
			errCnt += error[i];
		}
	}


	if (errCnt)
		printf("FAILED (%d issues)\n", errCnt);
	else
		printf("PASSED\n");

	delete pHtHif;

	return errCnt;
}

// Print usage message and exit with error.
void
usage(char *p)
{
	printf("usage: %s [count (default 100)] \n", p);
	exit(1);
}
