#include <str.h>
#if 0
int main()
{
	char buf[100];
	int len;
	
	char str1[]="The token   in this statement is separated  by   space";
	char str2[]="Tokens are not just words but characters separated by the character e";
	char str3[]="Here| everyting:||that separat|ed||by a pipe| is considered as tok|en";
	
	char str4[]="   a text for   trim testing  a  ";
	
	printf("str1 = %s\n", str1);
	printf("str2 = %s\n", str2);
	printf("str3 = %s\n", str3);
	printf("str3 = [%s]\n\n", str4);
	
	printf("str_total_characters(str1, space) = %d\n", str_total_characters(str1, ' '));
	printf("str_total_tokens(str1, space) = %d\n", str_total_tokens(str1, ' '));
	
	printf("str_total_characters(str2, e) = %d\n", str_total_characters(str2, 'e'));
	printf("str_total_tokens(str2, e) = %d\n", str_total_tokens(str2, 'e'));
	
	printf("str_get_token_info(str3, 2, '|', &len) = %p\n", str_get_token_info(str3, 2, '|', &len) );
	printf("returned len = %d\n", len);

	printf("str_get_token_info(str3, 3, '|', &len) = %p\n", str_get_token_info(str3, 3, '|', &len) );
	printf("returned len = %d\n", len);
	
	printf("str_get_token(buf, str3, 2, '|') = %s\n", str_get_token(buf, str3, 2, '|') );
	printf("str_get_token(buf, str3, 0, '|') = %s\n", str_get_token(buf, str3, 0, '|') );
	printf("str_get_token(buf, str3, 20, '|') = %s\n", str_get_token(buf, str3, 20, '|') );
	
	printf("str_ltrim(str4) = [%s]\n", str_ltrim(str4) );
	printf("str_rtrim(str4) = [%s]\n", str_rtrim(str4) );
	str_replace(str4, ' ', '|');
	printf("str_replace(str4, ' ', '|'); [%s]\n", str4 );
	
	printf("str_pattern_search(str1, \"*statement*\") = %d\n", str_pattern_search(str1, "*statement*") );
	printf("str_pattern_search(str2, \"*statement*\") = %d\n", str_pattern_search(str2, "*statement*") );
}
#endif
