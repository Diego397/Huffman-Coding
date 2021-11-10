//Diego Alejandro Balejo Cardozo Rey - 495908 - Trabalho 03
#include <iostream>
#include <sstream>
#include <fstream>
#include <queue>
#include <map>
#include <string>
#include <bitset>
#include <cstring>

struct node
{
	bool isLeaf;
	char value;

	node *leftChild, *rightChild;
};
//bitstream para entrada
struct ibitstream
{
	unsigned char available = 0;
	std::istream &s;
	bool binDump;
	char temp;

	ibitstream(std::istream &s, bool binDump = false) : s(s), binDump(binDump)
	{}

	bool getBit()
	{
		if(available == 0)
		{
			available = 8;
			f();
		}
		bool ret = ((temp >> 7) & 1);
		temp <<= 1;
		available--;
		return ret;
	}

	bool done()
	{
		if(available != 0)
			return false;
		if(binDump)
		{
			char c;
			while(s.peek() == ' ')
				s.get(c);
		}
		if(s.peek() != EOF)
			return false;
		return true;
	}

	void f()
	{
		if(!binDump)
		{
			s.get(temp);
		}
		else
		{
			char c;
			temp = 0;
			for(int i = 0; i < 8; ++i)
			{
				s >> c;
				temp <<= 1;
				temp += (c - '0');
			}
		}
	}
};
//bitstream para saida
struct obitstream
{
	unsigned char available = 8;
	std::ostream &s;
	bool binDump;
	char temp;

	obitstream(std::ostream &s, bool binDump = false) : s(s), binDump(binDump)
	{}

	void setBit(bool bit)
	{
		if(available == 0)
		{
			available = 8;
			f();
			temp = 0;
		}
		temp <<= 1;
		temp |= bit;
		available--;
	}

	void flush()
	{
		while(available != 0)
			available--, temp <<= 1;
		f();
	}

	void f()
	{
		if(!binDump)
		{
			s << temp;
		}
		else
		{
			for(int i = 7; i >= 0; --i)
				if(temp & (1 << i))
					s << '1';
				else
					s << '0';
			s << ' ';
		}
	}
};
//Definicao de NodeHeap
typedef std::priority_queue<std::tuple<unsigned int, char, node*>, std::vector<std::tuple<unsigned int, char, node*>>, std::greater<std::tuple<unsigned int, char, node*>>> NodeHeap;
//Funcao para ler uma string de um arquivo
std::string readFile (std::string filePath)
{   
    std::ifstream arq (filePath);
    if (!arq.is_open())
    {
        std::cerr << "Arquivo nao encontrado" << std::endl;
        return "";
    }
    
    std::stringstream strStream;
    strStream << arq.rdbuf();
    return strStream.str();
}
//Funcao para escrever uma string em um arquivo
void writeFile (std::string filePath, std::string fileContents)
{
	std::ofstream arq (filePath);
	if (!arq.is_open())
	{
		std::cerr << "Arquivo nao encontrado" << std::endl;
		return;
	}
	arq << fileContents;
}
//Funcao para salvar os caracteres e suas frequencias em um map
std::map<char, unsigned int> getFrequencies(const std::string &str)
{
	std::map<char, unsigned int> ret;
	for(const char &c : str)
		ret[c]++;
	return ret;
}
//Funcao para criar a arvore de huffman a partir de uma heap com mais de um no (mais de um caractere)
node *buildTreeFromHeap(NodeHeap &queue)
{
	while(queue.size() >= 2)
	{
		std::tuple<unsigned int, char, node*> a = queue.top();
		queue.pop();
		std::tuple<unsigned int, char, node*> b = queue.top();
		queue.pop();
		node *temp = new node();
		temp->leftChild = std::get<2>(a);
		temp->rightChild = std::get<2>(b);
		temp->isLeaf = false;
		temp->value = std::min(temp->leftChild->value, temp->rightChild->value);

		queue.emplace(std::get<0>(a) + std::get<0>(b), temp->value,temp);
	}

	return std::get<2>(queue.top());
}
//Funcao para criar a arvore de huffman a partir de uma heap com somente um no (apenas um caractere)
node *buildTreeFromHeapOne(NodeHeap &queue)
{
		std::tuple<unsigned int, char, node*> a = queue.top();
		std::tuple<unsigned int, char, node*> b = queue.top();
		queue.pop();
		node *temp = new node();
		temp->leftChild = std::get<2>(a);
		temp->rightChild = std::get<2>(b);
		temp->isLeaf = false;
		temp->value = std::min(temp->leftChild->value, temp->rightChild->value);

		queue.emplace(std::get<0>(a) + std::get<0>(b), temp->value,temp);

	return std::get<2>(queue.top());
}
//Funcao para criar a arvore de huffman a partir dos caracteres e suas frequencias
node *buildTreeFromFrequencies(const std::map<char, unsigned int> &frequencies, bool b)
{
	NodeHeap queue;

	for(const std::pair<char, unsigned int> &c : frequencies)
	{
		const char &value = c.first;
		const unsigned int &freq = c.second;

		node *temp = new node();
		temp->value = value;
		temp->leftChild = temp->rightChild = nullptr;
		temp->isLeaf = true;
		queue.emplace(freq, temp->value, temp);
	}
	//Se o arquivo tiver somente um caractere
	if(b)
		return buildTreeFromHeapOne(queue);

	return buildTreeFromHeap(queue);
}
//Funcao para liberar a memoria da arvore
void destroyTree(node *n, bool b)
{
	if(n->isLeaf)
	{
		delete n;
		return;
	}
	//Se o arquivo nao tiver somente um caractere
	if (!b)
		destroyTree(n->leftChild, b);
	destroyTree(n->rightChild, b);
	delete n;
}
//Funcao para mostrar os caracteres e seus codigos numa arvore
void printTree(node *n)
{
	static std::string path;
	if(n->isLeaf)
	{
		std::cout << path << ": " << n->value << std::endl;
		return;
	}
	path += "0";
	printTree(n->leftChild);
	path[path.size() - 1] = '1';
	printTree(n->rightChild);
	path.pop_back();
}
//Funcao para gerar os codigos de cada caractere
void generateCodingMap(node *n, std::map<char, std::string> &m)
{
	static std::string path;
	if(n->isLeaf)
	{
		m[n->value] = path;
		return;
	}
	path += "0";
	generateCodingMap(n->leftChild, m);
	path[path.size() - 1] = '1';
	generateCodingMap(n->rightChild, m);
	path.pop_back();
}
//Funcao para criar a arvore do preambulo de um arquivo codificado
void buildPreambleTree(node *n, obitstream &out)
{
	if (n->isLeaf)
	{
		out.setBit(1);

		std::string binChar = "";

		binChar = std::bitset<8>(n->value).to_string();

		for(const char &bit : binChar)
		{
			out.setBit(bit - '0');
		}
		return;
	}
	out.setBit(0);
	buildPreambleTree(n->leftChild, out);
	buildPreambleTree(n->rightChild, out);
}
//Funcao para ler a arvore do Preambulo de um arquivo codificado
node *readPreambleTree(ibitstream &in)
{
	bool b = in.getBit();

	if (b == 0)
	{	
		node *ret = new node();
		ret->isLeaf = false;
		ret->leftChild = readPreambleTree(in);
		ret->rightChild = readPreambleTree(in);
		return ret;
	}

	node *ret = new node();
	ret->isLeaf = true;

	std::string binChar = "";
	std::string s;
	
	for (int i = 0; i < 8; i++)
	{
		if (in.getBit())
			s = "1";
		else
			s = "0"; 
		binChar.append(s);
	}
	ret->value = char(std::bitset<8>(binChar).to_ullong());
	
	return ret;
}
//Funcao para codificar; gera e escreve os 32 primeiros bits que sao referentes a quantidade de nos da arvore; o preambulo; e o arquivo codificado 
void encode(std::string fileContents, obitstream &out)
{
	std::map<char, unsigned int> frequencies = getFrequencies(fileContents);

	if (fileContents == "")
	{
		return;
	}
	else
	{
		node *treeroot;
		if (frequencies.size() == 1)
			treeroot = buildTreeFromFrequencies(frequencies, 1);
		else
			treeroot = buildTreeFromFrequencies(frequencies, 0);

		std::map<char, std::string> characterToCoding;
		generateCodingMap(treeroot, characterToCoding);

		std::string binChar = "";

		unsigned int n =  fileContents.size();
		//Os primeiros 32 bits do arquivo sao referetes ao numero de nos da arvore
		binChar = std::bitset<32>(n).to_string();

		for(const char &bit : binChar)
		{
			out.setBit(bit - '0');
		}

		buildPreambleTree(treeroot, out);
		
		for(const char &c : fileContents)
		{
			for(const char &bit : characterToCoding[c])
			{
				out.setBit(bit - '0');
			}
		}

		out.flush();
		if (frequencies.size() == 1)
			destroyTree(treeroot, 1);
		else
			destroyTree(treeroot, 0);
	}
}
//Funcao para decodificar; decodifica os 32 primeiros bits sao para a quantidade de nos da arvore; a arvore; e escreve o conteudo do arquivo decodificado
std::string decode(ibitstream &in)
{
	unsigned int n = 0;

	//Os primeiros 32 bits sao referetes ao numero de nos da arvore
	for (int i = 0; i < 32; i++)
	{
		n <<= 1;
		n += in.getBit();
	}

	node *treeroot = readPreambleTree(in);

	std::string output;

	node *cur = treeroot;
	while(!in.done())
	{
		bool b = in.getBit();

		if(b == 0)
			cur = cur->leftChild;
		else
			cur = cur->rightChild;
		if(cur->isLeaf)
		{
			output += cur->value;
			cur = treeroot;
		}
		if (output.size() == n)
			break;
	}

	destroyTree(treeroot, 0);
	
	return output;
}
//Funcao para verificar se um arquivo esta vazio
bool is_empty(std::ifstream &file)
{
	return file.peek() == std::ifstream::traits_type::eof();
}
//Funcao main
int main(int argc, const char *argv[])
{
	//argv[2] = Entrada; argv[3] = Saida;
	if ((strcmp(argv[1], "c") == 0 || strcmp(argv[1], "C") == 0) && argc == 4)
	{
		std::cout << "Comprimindo...\n";
		
		std::ofstream f(argv[3]); //Cria ou abre o arquivo de saida
		if(!f.is_open())
		{
			std::cerr << "Failed to open file" << std::endl;
			exit(0);
		}

		obitstream bstream(f, false);
		encode(readFile(argv[2]), bstream); //Codifica a Entrada para o arquivo de saida

		f.close();
		
		std::cout << "Os dados do Arquivo : '" << argv[2] << "'\nForam comprimidos para o Arquivo : '" << argv[3] << "'\n";
	}
	else if ((strcmp(argv[1], "d") == 0 || strcmp(argv[1], "D") == 0) && argc == 4)
	{
		std::cout << "Descomprimindo...\n";
		
		std::ifstream f(argv[2]); //Abre o arquivo de entrada
		if(!f.is_open())
		{
			std::cerr << "Failed to open file" << std::endl;
			exit(0);
		}

		ibitstream bstream(f, false);

		if (is_empty(f))
			writeFile(argv[3], "");
		else
			writeFile(argv[3], decode(bstream)); //Decodifica a Entrada para o arquivo de saida

		f.close();

		std::cout << "Os dados do Arquivo : '" << argv[2] << "'\nForam descomprimidos para o Arquivo : '" << argv[3] << "'\n";
	}
	else
		std::cout << "Entrada Invalida" << std::endl;
}