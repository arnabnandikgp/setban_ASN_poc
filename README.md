# setban_ASN_poc
This is a proof of concept for the batch object proposed in this comment. There are also some discussions here.
I made the following decisions to simplify the implementation of this PoC:

making a new binary file to store the banned ASNs
API for retrieving and add data into the new binary file
Making relevent changes in the isBanned() API to allow bucketing for addrman.h
no API for decompressing tweak_check.
support only strauss_batch for batch verification.
no callback func and callback data.

To take the input of an ASN and make it clear that it is an ASN and not a ip address.
We can take motivation from the fact that the existing setban rpc takes into account that there are / in a subnet address and thus separates subnet masks and individual host addresses in the following ways.

So we can take the input and make an additional test and make a flag variable like for example isAS and if there are no dots in the parsed string then we can come to the conclusion that it is an ASN rather than a host address or a subnet mask
As follows:




Since there is already a banlist.dat in the local directory that stores the list of banned ip addresses it will be good to either integrate the data of banned ASNs into it or maybe make a new memory file to store the data of banned ASNs.
The above proposal is a bit bold as I know the fact that it will not be easy to get all the bitcoin developers to agreement on building a new local file to store the list of banned list of ASN .

There can be an another way too, that is using the bannedlist.dat used for storing the list of banned IPs to also store the the list of banned ASNs but I am not sure how to read and write two different categories of data from a single binary file.

To make this happen we need to make suitable API s for accessing the bannedasnlist.dat from the disk as well as make functions to modify its content.

Since in https://github.com/bitcoin/bitcoin/blob/master/src/util/asmap.cpp we have uint_32 datatype as output for the interpret function hence it will be suitable to stick to it and use the uint_32 datatype when handling ASNs.

Adding the following methods and objects in the banman class in order to ban ASNs using the asmap provided like changing the .isbanned() method to encompass functionalities like banning peers that have a given ASN.

We also need to implement the equivalent disconnectnode() method for our project as we need to ensure that after placing the ASN in the banned list if there are any peers belonging to that AS should be disconnected immediately.

Also calling the disconnectnode() method to immediately execute the ban request.

