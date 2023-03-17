# setban_ASN_poc
This is a proof of concept for the batch object proposed in this comment. There are also some discussions here.
I made the following decisions to simplify the implementation of this PoC:

making a new binary file to store the banned ASNs
API for retrieving and add data into the new binary file
Making relevent changes in the isBanned() API to allow bucketing for addrman.h
no API for decompressing tweak_check.
support only strauss_batch for batch verification.
no callback func and callback data.

