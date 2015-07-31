#include "QiParser.h"

/*QiParser::Line::Line(): base_type(line, "Line"){
	using qi::eol;
	using qi::eps;
	using qi::_val;
	using qi::char_;
	using qi::digit;
	using qi::alnum;
	using qi::double_;
	using qi::_1;

	time %= eps[_val=""]
		>> +digit[_val += _1] >> char_(':')[_val += _1]
		>> +digit[_val += _1] >> char_(':')[_val += _1]
		>> +digit[_val += _1];

	line %=
		alnum[_val = bind(&newByMove<string>, move(_1))] >> ',' >>
		digit[_val = bind(&newByMove<string>, move(_1))] >> ',' >>
		time[_val = bind(&newByMove<string>, move(_1))] >> ',' >>
		double_[_val = bind(&newByMove<const double*>, move(_1))] >> ',' >>
		double_[_val = bind(&newByMove<const double*>, move(_1))] >> ',' >>
		double_[_val = bind(&newByMove<const unsigned short *>, move(_1))] >> eol;
}*/

QiParser::QiParser(string filepath, AtomicQueue<Observation>* queue):
	m_mapped(filepath, 268435456, MemoryMapped::SequentialScan),
	m_queue(queue)
{}

QiParser::QiParser(QiParser&& other):
	m_mapped(move(other.m_mapped)),
	m_queue(move(other.m_queue))
{}

QiParser& QiParser::operator=(QiParser&& other){
	m_mapped = move(other.m_mapped);
	m_queue = move(other.m_queue);
	return *this;
}

void QiParser::run(){
	parse();
}

void QiParser::parse(){
	using qi::phrase_parse;
	using qi::eol;
	using qi::_1;
	using ascii::space;
	using phoenix::ref;
	using qi::eps;
	using qi::_val;
	using qi::char_;
	using qi::digit;
	using qi::alnum;
	using qi::alpha;
	using qi::repeat;
	using qi::double_;

	if(!m_mapped.isValid())
		return;

	//Observation obs;

	//Line line;
	//Observation obs;
	string line;

	//bool success = phrase_parse(m_mapped.begin() + 30, m_mapped.end(),
		/*(line[[&](){ref(obs) = _1; m_queue->enqueue(obs); }]),*/
		/*(
		repeat(5)[+alpha[phoenix::ref(line) += _1] >> ','] >> +alpha[phoenix::ref(line) += _1] >> eol
		),
		space);
	if(!success){
		_CrtDbgBreak();
	} else{
		cout << line << endl;
	}*/
}

template<typename T>
T* QiParser::newByMove(T&& org){
	static_assert(is_move_constructible<T>::value, "Type must be MoveConstructible !");
	return new T(org);
}