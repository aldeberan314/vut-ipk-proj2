# IPK - PROJEKT 2 - EPSILON (SFTP)

Zadaním projektu bola implementácia serveru a klienta protokolu SFTP (Simple File Transfer Protocol), ktorý je špecifikovaný v RFC 913. (https://datatracker.ietf.org/doc/html/rfc913)
Program sa skladá z dvoch spustitelných súborov, ipk-simpleftp-server a ipk-simpleftp-client, implementujúce server, respektíve klient. Server sa spustí ako prvý, následne sa na neho pripojí klient cez TCP. Klient zasiela serveru dotazy (špecifikované v dokumentácii manual.pdf), server ich vykonáva a zasiela klientovy odpovede. Ak neni pri spúštaní zadaný port, program bude načúvať na východzom porte 115. Podrobný popis sa nachádza v dokumente manual.pdf.


### Obmedzenia
- Server naslúcha vždy na všetkých rozhraniach. Výber špecifického rozhrania nie je podporovaný, pretože sa ho nepodarilo včas spolahlivo implemenotvať.
- Niektoré príkazy (ACCT / TYPE) sú implementované pre uplnosť a majú minimálnu funkcionality (detailne v manual.pdf)


### Zoznam súborov

- main.cpp
- mainc.cpp
- argparser_client.cpp / .h
- argparser.cpp / .h
- server.cpp / .h
- client.cpp / .h
- error.cpp / .h
- utils.cpp / .h
- hints.h
- manual.pdf
- README.md
- makefile