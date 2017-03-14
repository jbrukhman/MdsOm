#ifndef MdsOmOs_H
#define MdsOmOs_H

/** Parse a string into tokens, thread-safe version.
 * See the local OS man page for strtok for details.
 * @param buf The string to parse. Pass as value for 1st call and NULL to continue in the same string.
 * @param seps A string list of the separators to use(e.g., ",:" for commas and colons, default is whitespace).
 * @param last A ptr to a char* to hold current position.
 * @param returnSeps True means return the separators as tokens, false means not to, default is false.
 * @return char* the token, NULL when no more tokens.
 */
extern char* mds_strtok(char* buf, const char* seps, char** last, bool returnSeps = false);

/** Trim the leading and trailing whitespace from a string.
 * @param p The string to trim.
 * @return char* Ptr to trimmed string, using same storage as the original string.
 */
extern char* mds_trim(char* p);

#endif