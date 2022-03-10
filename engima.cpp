#include<bits/stdc++.h>
using namespace std;

class plugboard{
	public:
		void initialize(string input_codebook)
		{
			codebook = input_codebook;
			return;
		}
		
		char transform(char c)
		{
			const char* temp;
			temp = codebook.data();
			return temp[c - 'A'];
		}
		
	private:
		string codebook;
};

class rotor{
	public:
		void initialize(int input_index, char input_ringsetting, char input_messagekey, char input_keyword, string input_codebook)
		{
			index = input_index;
			ringsetting = input_ringsetting;
			messagekey = input_messagekey;
			keyword = input_keyword;
			codebook = input_codebook;
			return;
		}
		
		char positive_pass(char c)
		{
			int delta = (messagekey - ringsetting) % 26;
			const char* temp;
			temp = codebook.data();
			c = ((c - 'A') + delta + 26) % 26;
			c = (temp[c] - delta - 'A' + 26) % 26 + 'A';
			return c;
		}
		
		char negative_pass(char c)
		{
			int delta = (messagekey - ringsetting) % 26;
			c = ((c - 'A') + delta + 26) % 26 + 'A';
			return (codebook.find(c) - delta + 26) % 26 + 'A';
		}
		
		int rotate(int step)
		{
			messagekey = (messagekey + step - 'A') % 26 + 'A';
			if(index == 2 && messagekey == 'E')
				messagekey = (messagekey + 1 - 'A') % 26 + 'A';
			if(messagekey == keyword)
				return 1;
			else
				return 0;
		}
		
	private:
		char messagekey;
		char ringsetting;
		int index;
		char keyword;
		string codebook;	
};

class reflect{
	public:
		void initialize(string input_codebook)
		{
			codebook = input_codebook;
			return;
		}
		
		char transform(char c)
		{
			const char* temp;
			temp = codebook.data();
			return temp[(c - 'A' + 26) % 26];
		}
		
	private:
		string codebook;
};

class enigma{
	public:
		void initialize(string ringsetting, string messagekey, string firstcodebook, string secondcodebook, string thirdcodebook, string plugboard)
		{
			const char* temp_messagekey;
			const char* temp_ringsetting;
			temp_messagekey = messagekey.data();
			temp_ringsetting = ringsetting.data();
			r3.initialize(3, temp_ringsetting[0], temp_messagekey[0], 'R', firstcodebook);
			r2.initialize(2, temp_ringsetting[1], temp_messagekey[1], 'F', secondcodebook);
			r1.initialize(1, temp_ringsetting[2], temp_messagekey[2], 'A', thirdcodebook);
			p1.initialize(plugboard);
			f1.initialize("YRUHQSLDPXNGOKMIEBFZCWVJAT");
			return;
		}
		
		char encrypt(char c)
		{
			r3.rotate(r2.rotate(r1.rotate(1)));
			c = p1.transform(c);
			c = r1.positive_pass(c);
			c = r2.positive_pass(c);
			c = r3.positive_pass(c);
			c = f1.transform(c);
			c = r3.negative_pass(c);
			c = r2.negative_pass(c);
			c = r1.negative_pass(c);
			c = p1.transform(c);
		}
		
	private:
		plugboard p1;
		rotor r1, r2, r3;
		reflect f1;	
};

int main()
{
	char c;
	enigma e1;
	string messagekey, ringsetting;
	cin >> ringsetting;
	cin >> messagekey;
	e1.initialize(ringsetting, messagekey, "EKMFLGDQVZNTOWYHXUSPAIBRCJ", "AJDKSIRUXBLHWTMCQGZNPYFVOE", "VZBRGITYUPSDNHLXAWMJQOFECK", "AGDCRVBNUKJMLHPOQESYIFWXTZ");
	
	while(1)
	{
		cin >> c;
		cout << e1.encrypt(c);
	}
	return 0;
}
