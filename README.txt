 .----------------.  .----------------.  .----------------.  .----------------.                     
| .--------------. || .--------------. || .--------------. || .--------------. |                    
| |     ______   | || |      __      | || |  _______     | || |  ________    | |                    
| |   .' ___  |  | || |     /  \     | || | |_   __ \    | || | |_   ___ `.  | |                    
| |  / .'   \_|  | || |    / /\ \    | || |   | |__) |   | || |   | |   `. \ | |                    
| |  | |         | || |   / ____ \   | || |   |  __ /    | || |   | |    | | | |                    
| |  \ `.___.'\  | || | _/ /    \ \_ | || |  _| |  \ \_  | || |  _| |___.' / | |                    
| |   `._____.'  | || ||____|  |____|| || | |____| |___| | || | |________.'  | |                    
| |              | || |              | || |              | || |              | |                    
| '--------------' || '--------------' || '--------------' || '--------------' |                    
 '----------------'  '----------------'  '----------------'  '----------------'                     
 .----------------.  .----------------.  .----------------.  .----------------.  .----------------. 
| .--------------. || .--------------. || .--------------. || .--------------. || .--------------. |
| |     ______   | || |   _____      | || |      __      | || |    _______   | || |  ____  ____  | |
| |   .' ___  |  | || |  |_   _|     | || |     /  \     | || |   /  ___  |  | || | |_   ||   _| | |
| |  / .'   \_|  | || |    | |       | || |    / /\ \    | || |  |  (__ \_|  | || |   | |__| |   | |
| |  | |         | || |    | |   _   | || |   / ____ \   | || |   '.___`-.   | || |   |  __  |   | |
| |  \ `.___.'\  | || |   _| |__/ |  | || | _/ /    \ \_ | || |  |`\____) |  | || |  _| |  | |_  | |
| |   `._____.'  | || |  |________|  | || ||____|  |____|| || |  |_______.'  | || | |____||____| | |
| |              | || |              | || |              | || |              | || |              | |
| '--------------' || '--------------' || '--------------' || '--------------' || '--------------' |
 '----------------'  '----------------'  '----------------'  '----------------'  '----------------' 
----------------------------------------------------------------------------------------------------------

CARD CLASH

by 9~bit - CSD1451s25 Session B	Spring 2025
	Yar Xuan				xuan.yar@digipen.edu
	Edthan Yeo				xiongyuedthan.yeo@digipen.edu
	Low Jianlin				jianlin.low@digipen.edu
	Xiong Yang				xiong.yang@digipen.edu

----------------------------------------------------------------------------------------------------------

HOW TO PLAY / CONTROLS

During your turn, you can drag and drop the cards to manually re-arrange their positions.
You can also click on the shuffle and sort buttons on the right to shuffle their positions or sort by ascending order.

Turn phase 1 - Draw card from opponent or power card deck

Left click on a card on the opponent side to draw a card from them.
Left click on power card deck to draw a power card.


Turn phase 2 - Play a power card or a set of 4, one of a kind number cards

Left click on a power card or number card to select it.
If a power card is selected, right-click to play it.
If a set of 4 numbers is selected, right-click to play it.
If you have no cards to play, you can skip turn by pressing space.

Win Condition - Be the first player to empty their hand

----------------------------------------------------------------------------------------------------------

NOTE

Card Clash requires 1 server and 2 clients to be run for play testing on your local computer.

The clients are currently hardcoded to connect to the local server IP 127.0.0.1 for easier testing. 
This can be changed in CardClash/Source/SocketUtils.cpp.

To run the server and client through Visual Studio, the solution's startup projects configuration needs to be changed:

Right click solution -> Configure Startup Projects
Change to Multiple startup projects:
Add the Server project, and change the sequence so that it's 
Server then CardClash. This will ensure the server starts up first
then the client.

After that, the debugger should start both the server and client.
The 2nd client can be opened up through the CardClash.exe file in
bin/Debug-x64

----------------------------------------------------------------------------------------------------------

CREDITS

Group Members:
Yar Xuan, Low Jian Lin, Xiong Yang, Edthan Yeo

Instructors:
Cheng Ding Xiang, Gerald Wong, Soroor Faradounbeh

Created at:
DigiPen Institute of Technology Singapore

President:
Claude Comair

Executives:
Claude Comair, Chu Jason Yeu Tat, Michael Gats
Tan Chek Ming, Prasanna Kumar Ghali, Mandy Wong, Johnny Deek

