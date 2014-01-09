Protocolo
=========

Protocolo de Ligação deDados

Com a elaboração deste trabalho pretendíamos implementar um
protocolo de ligação de dados de acordo com um conjunto de
especificações, de modo a fornecer um serviço de comunicação de dados
fiável entre dois sistemas ligados por um canal de transmissão, neste caso,
um cabo de série.
De modo a verificar que o protocolo foi correctamente implementado,
criámos uma simples aplicação de transferência de ficheiros. Quer o
protocolo, quer a aplicação de transferência foram desenvolvidos com
recurso a uma máquina com
Linux
, usando como linguagem de
programação
Linguagem C
.
Neste relatório procuramos apresentar a nossa implementação do
referido protocolo, o qual tem como esquema arquitectónico o que a seguir
se ilustra. Seguidamente, apresentamos a estrutura do código e as
principais funcionalidades do protocolo, bem como as estratégias de
implementação usadas.
Na parte final, destacamos os testes realizados por forma a validar os
resultados esperados e referimos as nossas conclusões acerca do projecto
realizado
