#include <winscard.h>
#include <iostream>
#include <cstdlib>


#pragma comment(lib, "Winscard.lib")

#ifdef WIN32
static char* pcsc_stringify_error(LONG rv)
{
	static char out[20];
	sprintf_s(out, sizeof(out), "0x%08X", rv);

	return out;
}
#endif

#define CHECK(f, rv) \
 if (SCARD_S_SUCCESS != rv) \
 { \
  printf(f ": %s\n", pcsc_stringify_error(rv)); \
  return -1; \
 }

using std::cout;
using std::endl;


int main() {
	LONG rv;
	
	SCARDCONTEXT hContext;
	LPTSTR mszReaders;
	SCARDHANDLE hCard;
	DWORD dwReaders, dwActiveProtocol, dwRecvLength;

	SCARD_IO_REQUEST pioSendPci;
	BYTE pbRecvBuffer[258];

	BYTE cmd1[] = { 0xff, 0xd6, 0x00, 0x00, 0x0b, 0x48, 0x65, 0x6c, 0x6c,
		0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64 };

	unsigned int i;

	rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
	CHECK("SCardEstablishContext", rv);
#ifdef SCARD_AUTOALLOCATE
	dwReaders = SCARD_AUTOALLOCATE;
	rv = SCardListReaders(hContext, NULL, (LPTSTR) &mszReaders, &dwReaders);
	CHECK("SCardListReaders", rv);
#else
	rv = SCardListReaders(hContext, NULL, NULL, &dwReaders);
	CHECK("SCardListReaders", rv);

	mszReaders = calloc(dwReaders, sizeof(char));
	rv = SCardListReaders(hContext, NULL, mszReaders, &dwReaders);
	CHECK("SCardListReaders", rv);
#endif

	cout << "Reader name: " << mszReaders << endl;

	rv = SCardConnect(hContext, mszReaders, SCARD_SHARE_SHARED,
		SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);
	
	switch (dwActiveProtocol) {
	case SCARD_PROTOCOL_T0:
		pioSendPci = *SCARD_PCI_T0;
		break;
	case SCARD_PROTOCOL_T1:
		pioSendPci = *SCARD_PCI_T1;
		break;

	}
	
	dwRecvLength = sizeof(pbRecvBuffer);
	rv = SCardTransmit(hCard, &pioSendPci, cmd1, sizeof(cmd1),
		NULL, pbRecvBuffer, &dwRecvLength);
	CHECK("SCardTransmit", rv)

		printf("response: ");
	for (i = 0; i < dwRecvLength; i++)
		printf("%02X ", pbRecvBuffer[i]);
	printf("\n");

	/*dwRecvLength = sizeof(pbRecvBuffer);
	rv = SCardTransmit(hCard, &pioSendPci, cmd2, sizeof(cmd2),
		NULL, pbRecvBuffer, &dwRecvLength);
	CHECK("SCardTransmit", rv)

		printf("response: ");
	for (i = 0; i < dwRecvLength; i++)
		printf("%02X ", pbRecvBuffer[i]);
	printf("\n");
*/
	rv = SCardDisconnect(hCard, SCARD_LEAVE_CARD);
	CHECK("SCardDisconnect", rv)

#ifdef SCARD_AUTOALLOCATE
		rv = SCardFreeMemory(hContext, mszReaders);
	CHECK("SCardFreeMemory", rv)

#else
		free(mszReaders);
#endif

	rv = SCardReleaseContext(hContext);

	CHECK("SCardReleaseContext", rv)

		return 0;

	return EXIT_SUCCESS;
}