#include<stdio.h>
#include<string.h>

#define MAX 1000
#define left 0
#define mid 1
#define right 2

char reflector[]="YRUHQSLDPXNGOKMIEBFZCWVJAT"; 
char rotor_table[5][27] = 
{
	"EKMFLGDQVZNTOWYHXUSPAIBRCJ",
   	"AJDKSIRUXBLHWTMCQGZNPYFVOE",
    "BDFHJLCPRTXVZNYEIWGAKMUSQO",
    "ESOVPZJAYQUIRHXLNFTGKDCMWB",
    "VZBRGITYUPSDNHLXAWMJQOFECK"
};
char step_char[]="RFWKA";
char plugboard[26];

void get_PlugBoard(char* plugboard);
void rotate(char* messagekey, int* rotors);
char pass_reflector(char ch);
char pass_plugboard(char ch);
char positive_pass(char input, int RotorNum, char messagekey, char ringsetting);
char negative_pass(char input, int RotorNum, char messagekey, char ringsetting);
char encrypt(char ch, int* rotors, char* messagekey, char* ringsetting);
int string_find(char* answer, char* known_word);
void print_out(char* test_messagekey, char* answer, int* rotors);
int main()
{
	char ringsetting[4];
	char question[MAX];
    char answer[MAX];
	char known_word[MAX];

    int rotors[3];
    char temp_messagekey[3];
	char test_messagekey[3];
	int i;
    
	get_PlugBoard(plugboard);
	scanf("%s",ringsetting);
	scanf("%s",question);
	scanf("%s",known_word);
	scanf("%d",&rotors[left]);
    
	rotors[left]--;
	
	for(test_messagekey[left] = 'Z'; test_messagekey[left] >= 'A'; test_messagekey[left]--)
    {
		for(test_messagekey[mid] = 'A'; test_messagekey[mid] <= 'Z'; test_messagekey[mid]++)
        {
			for(test_messagekey[right] = 'A'; test_messagekey[right] <= 'Z'; test_messagekey[right]++)
            {
				for(rotors[mid] = 0; rotors[mid] < 5; rotors[mid]++)
                {
					if(rotors[mid] == rotors[left])
						continue;

					for(rotors[right] = 0; rotors[right] < 5; rotors[right]++)
                    {
						if(rotors[right] ==rotors[left] || rotors[right] == rotors[mid])
							continue;

						temp_messagekey[right] = test_messagekey[right];
						temp_messagekey[mid] = test_messagekey[mid];
						temp_messagekey[left] = test_messagekey[left];
                        
                        for(i = 0; question[i] != '\0'; i++)
                        {
                            rotate(temp_messagekey, rotors);
                            answer[i] = encrypt(question[i], rotors, temp_messagekey, ringsetting);
                        }
                        answer[i] = '\0';

						if(string_find(answer, known_word))
                        {
                            print_out(test_messagekey, answer, rotors);
                            goto end;
                        }
					}
				}
			}
		}
	}
    end:
	return 0;
}

void get_PlugBoard(char* plugboard)
{
	int i;
	for(i = 0; i < 26; i++)
		plugboard[i] = i + 'A';

	for(i = 0; i < 10; i++)
	{
		char temp[10];
		scanf("%s",temp);
		int j;
		for(j = 0; j < 26; j++)
		{
			if(plugboard[j] == temp[0])
			{
				plugboard[j] = temp[1];
				continue;
			}

			if(plugboard[j] == temp[1])
			{
				plugboard[j] = temp[0];
				continue;
			}
		}
	}
	return;
}

void rotate(char* messagekey, int* rotors)
{
    messagekey[right] = (messagekey[right] - 'A' + 1) % 26 + 'A';

	if(messagekey[right] == step_char[rotors[right]])
		messagekey[mid] = (messagekey[mid] - 'A' + 1) % 26 + 'A';

	else if(messagekey[mid] == (step_char[rotors[mid]] - 'A' + 25) % 26 + 'A')
    {
		messagekey[mid] = (messagekey[mid] - 'A' + 1) % 26 + 'A';
		messagekey[left] = (messagekey[left] - 'A' + 1) % 26 + 'A';
	}
    return;
}

char pass_plugboard(char ch)
{
    return plugboard[ch - 'A'];
}

char pass_reflector(char ch)
{
    return reflector[ch - 'A'];
}

char positive_pass(char ch, int rotor, char messagekey, char ringsetting)
{
    int delta = messagekey - ringsetting;
	ch = (ch - 'A' + delta + 26) % 26 + 'A';
	ch = rotor_table[rotor][ch - 'A'];
	ch = (ch - 'A' - delta + 26) % 26 + 'A';
	return ch;
}

char negative_pass(char ch, int rotor, char messagekey, char ringsetting)
{
	int i;
    int delta = messagekey - ringsetting;
	ch = (ch - 'A' + delta + 26) % 26 + 'A';
	for(i = 0; i < 26; i++)
    {
		if(rotor_table[rotor][i] == ch)
        {
			ch = 'A' + i;
			break;
		}
	}
	ch = (ch - 'A' - delta + 26) % 26 + 'A';
    return ch;
}

char encrypt(char ch, int* rotors, char* messagekey, char* ringsetting)
{
    ch = pass_plugboard(ch);
	ch = positive_pass(ch, rotors[right], messagekey[right], ringsetting[right]);
	ch = positive_pass(ch, rotors[mid], messagekey[mid], ringsetting[mid]);
	ch = positive_pass(ch, rotors[left], messagekey[left], ringsetting[left]);
	ch = pass_reflector(ch);
	ch = negative_pass(ch, rotors[left], messagekey[left], ringsetting[left]);
	ch = negative_pass(ch, rotors[mid], messagekey[mid], ringsetting[mid]);
	ch = negative_pass(ch, rotors[right], messagekey[right], ringsetting[right]);
	ch = pass_plugboard(ch);
    return ch;
}

int string_find(char* answer, char* known_word)
{
    int i, j;
    for(i = 0; i <= strlen(answer) - strlen(known_word); i++)
    {
		if(answer[i] == known_word[0])
        {
            j = 0;
            while(answer[i] == known_word[j]&&known_word[j] != '\0')
            {
                i++;
                j++; 
            }
            if(known_word[j] == '\0')
                return 1;
	    }
	}
    return 0;
}

void print_out(char* test_messagekey, char* answer, int* rotors)
{
    printf("MessageKey=%c%c%c\n",test_messagekey[left], test_messagekey[mid], test_messagekey[right]);
    printf("PlainText=%s\n", answer);
    printf("RotorNum=%d%d%d",rotors[left] + 1, rotors[mid] + 1, rotors[right] + 1);
    return;
}