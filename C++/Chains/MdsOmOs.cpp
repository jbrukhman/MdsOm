

#include "MdsOmChains.h"

//namespace MdsOmNs {

// ----------------------------------------------------------------------
char *trim(char *str)
{
	// Trim leading space
	while (isspace(*str)) str++;

	if (*str == '\0') // All spaces?
		return str;

	// Trim trailing space
	char *end = str + strlen(str) - 1;
	while (end > str && isspace(*end)) end--;

	// Write new null terminator
	*(end+1) = '\0';

	return str;
}

// ----------------------------------------------------------------------
pair<string, string> getSourceAndSymbol(const char* topic)
{
	pair<string, string> ret;

	if (topic == NULL) return ret;

	const char delim = '.';	
	char* p = strdup(topic);
	p = trim(p);
	char* cp = strchr(p, delim);
	if (cp != NULL) {
		*cp = '\0';
		ret.first = p;
		ret.second = ++cp;
	}
	free(p);
	return ret;
}

// ------------------------------------------------------------------------
vector<string> mds_tokenize(const string& str, const string& delimiters)
{
	vector<string> tokens;
	 
	string::size_type lastPos = 0, pos = 0; 
	int count = 0;
	 
	if (str.length() < 1) return tokens;
	 
	// skip delimiters at beginning. 
	lastPos = str.find_first_not_of(delimiters, 0);

	if ((str.substr(0, lastPos-pos).length()) > 0) { 	
 		count =(int) str.substr(0, lastPos-pos).length(); 	

 		for (int i=0; i < count; i++) 	
 	 		tokens.push_back("");
	 	
 		if (string::npos == lastPos)
 			tokens.push_back("");
	}

	// find first "non-delimiter".
	pos = str.find_first_of(delimiters, lastPos);
	 
	while (string::npos != pos || string::npos != lastPos) { 
	 	// found a token, add it to the vector.
	 	tokens.push_back( str.substr(lastPos, pos - lastPos));
				
		// skip delimiters. Note the "not_of"
	 	lastPos = str.find_first_not_of(delimiters, pos);
		
		if ((string::npos != pos) &&(str.substr(pos, lastPos-pos).length() > 1)) {
 			count =(int) str.substr(pos, lastPos-pos).length();

 			for (int i=0; i < count; i++)
 	 			tokens.push_back("");
		}
		
 		pos = str.find_first_of(delimiters, lastPos);
	}

	return tokens;
}

//}
