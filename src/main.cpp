#include <stdio.h>
#include <stdlib.h>
#include <common.h>
#include <cdata.h>


int main(int argc, char *argv[])
{
	printf("@@@@@@@@@@ hello cstock @@@@@@@@@@ \n");
	cdata* data;
	data = cdata_new();
	if (data)
	{
		//data->ops->get_foreign_investor_sorting_data(data, 104, 8, 8);
		data->ops->update_data(data);
		data->ops->close(data);
	}
	
/*
	chttp* 	c;
	char* 	data_buf;
	int		read_size; 
	
	data_buf = (char*)malloc(PAGE_SIZE);
	
	c = chttp_new();
	c->ops->connect(c, "www.twse.com.tw");
	c->ops->post(c, "/ch/trading/fund/TWT38U/TWT38U.php", "download=csv&qdate=104%2F08%2F06&sorting=by_issue", data_buf, PAGE_SIZE, &read_size);
	c->ops->close(c);

	FILE *fp = fopen("data.html", "wb+");
    fwrite(data_buf, 1, read_size, fp);
    fclose(fp);
	
	free(data_buf);*/
	return 1;
}
