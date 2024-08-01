/*
 * (c) FFRI Security, Inc., 2024 / Author: FFRI Security, Inc.
 */

CHAR16*
StriStri(
    CHAR16* String,
    CHAR16* SearchString
    )
{
  CHAR16 buf1[256] = {0};
  CHAR16 buf2[256] = {0};

  int i;
  for(i=0; String[i]!='\0'; i++)
    buf1[i] = CharToUpper(String[i]);
  for(i=0; SearchString[i]!='\0'; i++)
    buf2[i] = CharToUpper(SearchString[i]);

  return StrStr(buf1, buf2);
}
