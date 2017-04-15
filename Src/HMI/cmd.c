#include "cmd.h"

static   char key;
static   uint8_t channel = 0;
static   uint8_t index = 0;

/**@brief  字符交互界面之help
 *
 * @detail 打印出字符交互界面的使用说明
 *
 */
void cmd_h(void)
{
    SEGGER_RTT_printf(0,"\r\nAfter \"[loop xx]\" is printed, press:\r\n");
    SEGGER_RTT_printf(0,"\r\nh\tprint this help information.\r\n");
    SEGGER_RTT_printf(0,    "l\tload valve params.\r\n");
    SEGGER_RTT_printf(0,    "s\tstore valve params.\r\n");
    SEGGER_RTT_printf(0,    "p\tprint valve params.\r\n");
    SEGGER_RTT_printf(0,    "m\tmodefy valve params.\r\n");
    SEGGER_RTT_printf(0,    "c\tchange channel state manually.\r\n");
    SEGGER_RTT_printf(0,    "i\tinsert a coder event manually.\r\n");
    SEGGER_RTT_printf(0,    "r\tremove a coder event manually.\r\n");
    SEGGER_RTT_printf(0,    "g\tgather coder event from valve params.\r\n");
    SEGGER_RTT_printf(0,    "d\tdisplay coder event chain.\r\n");
}

/**@breif  读取通道参数的命令函数
 */
void cmd_l(void)
{
    valve_params_load();
}

/**@brief  存储通道参数的命令函数
 */
void cmd_s(void)
{
    valve_params_store();
}

/**@brief  打印通道参数的命令函数
 *
 * @note   进入函数后会提示通过敲击1～9, a~c 按键来指定某一通道
 *
 */
void cmd_p(void)
{
    valve_display_t valve_display;

    SEGGER_RTT_printf(0,"\r\nChannel:");
    key = SEGGER_RTT_WaitKey();
    if('1'<=key&&key<='9')
            channel = key - '0';
    else if('a'<=key&&key<='c')
            channel = key - 'a' + 10;
    else if('A'<=key&&key<='C')
            channel = key - 'A' + 10;
    else
            return;
    valve_params_display(channel,&valve_display);
    SEGGER_RTT_printf(0,"\r\non_offs\r\n");
    for(index=0;index<ON_OFF_MAX;index++)
    {
            //SEGGER_RTT_printf(0,"\r\non_offs[%d]\r\n",index);
            SEGGER_RTT_printf(0,"%1d:\t%3d.%3d~%3d.%3d\t",
        		    index,
        		    valve_display.on_degree[index],
        		    valve_display.on_fraction[index],
        		    valve_display.off_degree[index],
        		    valve_display.off_fraction[index]);
            if((valve_display.on_offs_mask>>index)&0x0001)
        	    SEGGER_RTT_printf(0,"Y\r\n");
            else
        	    SEGGER_RTT_printf(0,"-\r\n");
    }
    SEGGER_RTT_printf(0,"high_duration\r\n");
    SEGGER_RTT_printf(0,"high:\t%3d.%3d\r\n",
        	    valve_display.high_degree,
        	    valve_display.high_fraction);
}

/**@brief  修改通道参数的命令函数
 *
 * @note   参数做修改的通道和新参数会在进入函数后提示输入
 *
 */
void cmd_m(void)
{
    valve_modify_t  valve_modify;

    SEGGER_RTT_printf(0,"\r\nChannel:");
    key = SEGGER_RTT_WaitKey();
    if('1'<=key&&key<='9')
            channel = key - '0';
    else if('a'<=key&&key<='c')
            channel = key - 'a' + 10;
    else if('A'<=key&&key<='C')
            channel = key - 'A' + 10;
    else
            return;
    SEGGER_RTT_printf(0,"\r\non_off_index:");
    valve_modify.on_off_index = SEGGER_RTT_WaitKey() - '0';
    SEGGER_RTT_printf(0,"\r\non_degree:");
    valve_modify.on_degree  = SEGGER_RTT_WaitKey() - '0';
    valve_modify.on_degree *= 10;
    valve_modify.on_degree += SEGGER_RTT_WaitKey() - '0';
    valve_modify.on_degree *= 10;
    valve_modify.on_degree += SEGGER_RTT_WaitKey() - '0';
    SEGGER_RTT_printf(0,"\r\non_fraction:");
    valve_modify.on_fraction  = SEGGER_RTT_WaitKey() - '0';
    valve_modify.on_fraction *= 10;
    valve_modify.on_fraction += SEGGER_RTT_WaitKey() - '0';
    valve_modify.on_fraction *= 10;
    valve_modify.on_fraction += SEGGER_RTT_WaitKey() - '0';
    SEGGER_RTT_printf(0,"\r\noff_degree:");
    valve_modify.off_degree  = SEGGER_RTT_WaitKey() - '0';
    valve_modify.off_degree *= 10;
    valve_modify.off_degree += SEGGER_RTT_WaitKey() - '0';
    valve_modify.off_degree *= 10;
    valve_modify.off_degree += SEGGER_RTT_WaitKey() - '0';
    SEGGER_RTT_printf(0,"\r\noff_fraction:");
    valve_modify.off_fraction  = SEGGER_RTT_WaitKey() - '0';
    valve_modify.off_fraction *= 10;
    valve_modify.off_fraction += SEGGER_RTT_WaitKey() - '0';
    valve_modify.off_fraction *= 10;
    valve_modify.off_fraction += SEGGER_RTT_WaitKey() - '0';
    SEGGER_RTT_printf(0,"\r\non_off_valid:");
    key = SEGGER_RTT_WaitKey();
    if(key=='Y'||key=='y'||key=='1')
            valve_modify.on_off_valid = 1;
    else
            valve_modify.on_off_valid = 0;
    SEGGER_RTT_printf(0,"\r\nhigh_valid:");
    key = SEGGER_RTT_WaitKey();
    if(key=='Y'||key=='y'||key=='1')
    {
            valve_modify.high_valid = 1;
            SEGGER_RTT_printf(0,"\r\nhigh_degree:");
            valve_modify.high_degree  = SEGGER_RTT_WaitKey() - '0';
            valve_modify.high_degree *= 10;
            valve_modify.high_degree += SEGGER_RTT_WaitKey() - '0';
            valve_modify.high_degree *= 10;
            valve_modify.high_degree += SEGGER_RTT_WaitKey() - '0';
            SEGGER_RTT_printf(0,"\r\nhigh_fraction:");
            valve_modify.high_fraction  = SEGGER_RTT_WaitKey() - '0';
            valve_modify.high_fraction *= 10;
            valve_modify.high_fraction += SEGGER_RTT_WaitKey() - '0';
            valve_modify.high_fraction *= 10;
            valve_modify.high_fraction += SEGGER_RTT_WaitKey() - '0';
    }
    else
            valve_modify.high_valid = 0;
    valve_params_modify(channel,valve_modify);
}

/**@brief  手动修改通道状态的命令函数
 *
 * @note   要修改状态的通道和新的状态会在进入函数后提示输入
 *
 */
void cmd_c(void)
{
    SEGGER_RTT_printf(0,"\r\nChannel:");
    key = SEGGER_RTT_WaitKey();
    if('1'<=key&&key<='9')
            channel = key - '0';
    else if('a'<=key&&key<='c')
            channel = key - 'a' + 10;
    else if('A'<=key&&key<='C')
            channel = key - 'A' + 10;
    else
            return;
    SEGGER_RTT_printf(0,"\r\nOn press O, Off press X:");
    key = SEGGER_RTT_WaitKey();  //等待输入
    if(key == 'o' || key == 'O')
            valve_channel_on(channel);
    else if(key == 'x' || key == 'X')
    {
            SEGGER_RTT_printf(0,"\r\nHigh press H, Low press L:");
            key = SEGGER_RTT_WaitKey();
            if(key == 'h' || key == 'H')
        	    valve_channel_off(channel,1);
            else if(key == 'l' || key == 'L')
        	    valve_channel_off(channel,0);
            else
        	    return;  //error to choose High or Low
    }
    else
            return;  //error to choose On or Off
}

/**@brief  插入一个编码器事件的命令函数
 *
 * @note   要插入的事件在进入函数后会提示输入
 *
 */
void cmd_i(void)
{
    coder_evt_t coder_evt;

    SEGGER_RTT_printf(0,"\r\nChannel:");
    key = SEGGER_RTT_WaitKey();
    if('1'<=key&&key<='9')
            coder_evt.evt_channel = key - '0';
    else if('a'<=key&&key<='c')
            coder_evt.evt_channel = key - 'a' + 10;
    else if('A'<=key&&key<='C')
            coder_evt.evt_channel = key - 'A' + 10;
    else
            return;
    SEGGER_RTT_printf(0,"\r\nOn press O, Off press X:");
    key = SEGGER_RTT_WaitKey();  //等待输入
    if(key == 'o' || key == 'O')
            coder_evt.evt_type = channel_on;
    else if(key == 'x' || key == 'X')
    {
            SEGGER_RTT_printf(0,"\r\nHigh press H, Low press L:");
            key = SEGGER_RTT_WaitKey();
            if(key == 'h' || key == 'H')
        	    coder_evt.evt_type = channel_off_H;
            else if(key == 'l' || key == 'L')
        	    coder_evt.evt_type = channel_off_L;
            else
        	    return;  //error to choose High or Low
    }
    else
            return;  //error to choose On or Off
    SEGGER_RTT_printf(0,"\r\nCoder_count:");
    coder_evt.coder_count  = SEGGER_RTT_WaitKey() - '0';
    coder_evt.coder_count *= 10;
    coder_evt.coder_count += SEGGER_RTT_WaitKey() - '0';
    coder_evt.coder_count *= 10;
    coder_evt.coder_count += SEGGER_RTT_WaitKey() - '0';
    coder_evt.coder_count *= 10;
    coder_evt.coder_count += SEGGER_RTT_WaitKey() - '0';
    coder_evt_insert(coder_evt);
}

/**@brief  移除一个编码器事件的命令函数
 *
 * @note    要移除的事件在进入函数后会提示输入
 *
 */
void cmd_r(void)
{
    coder_evt_t coder_evt;

    SEGGER_RTT_printf(0,"\r\nChannel:");
    key = SEGGER_RTT_WaitKey();
    if('1'<=key&&key<='9')
            coder_evt.evt_channel = key - '0';
    else if('a'<=key&&key<='c')
            coder_evt.evt_channel = key - 'a' + 10;
    else if('A'<=key&&key<='C')
            coder_evt.evt_channel = key - 'A' + 10;
    else
            return;
    SEGGER_RTT_printf(0,"\r\nOn press O, Off press X:");
    key = SEGGER_RTT_WaitKey();  //等待输入
    if(key == 'o' || key == 'O')
            coder_evt.evt_type = channel_on;
    else if(key == 'x' || key == 'X')
    {
            SEGGER_RTT_printf(0,"\r\nHigh press H, Low press L:");
            key = SEGGER_RTT_WaitKey();
            if(key == 'h' || key == 'H')
        	    coder_evt.evt_type = channel_off_H;
            else if(key == 'l' || key == 'L')
        	    coder_evt.evt_type = channel_off_L;
            else
        	    return;  //error to choose High or Low
    }
    else
            return;  //error to choose On or Off
    SEGGER_RTT_printf(0,"\r\nCoder_count:");
    coder_evt.coder_count  = SEGGER_RTT_WaitKey() - '0';
    coder_evt.coder_count *= 10;
    coder_evt.coder_count += SEGGER_RTT_WaitKey() - '0';
    coder_evt.coder_count *= 10;
    coder_evt.coder_count += SEGGER_RTT_WaitKey() - '0';
    coder_evt.coder_count *= 10;
    coder_evt.coder_count += SEGGER_RTT_WaitKey() - '0';
    coder_evt_remove(coder_evt);
}

/**@brief  从通道参数表汇集所有编码器事件的命令函数
 */
void cmd_g(void)
{
    coder_evt_gather();
}

/**@brief  显示编码器事件的命令函数
 */
void cmd_d(void)
{
    extern coder_evt_t * pcoder_evt_first;
    coder_evt_t * p_coder_evt = pcoder_evt_first;

    while(p_coder_evt != NULL)
    {
	SEGGER_RTT_printf(0,"\r\n%4d,\t%2d,\t%1d",
			  p_coder_evt->coder_count,
			  p_coder_evt->evt_channel,
			  p_coder_evt->evt_type);
	p_coder_evt = p_coder_evt -> next;
    }
}

/**@brief  字符交互界面
 *
 * @detail 通过JLinkRTT输入输出，敲击提示的一些按键可以配置参数、控制运转
 *
 */
void cmd_main(void)
{
    key = SEGGER_RTT_WaitKey();  //等待输入

    switch(key)
    {
        case 'h':
	case 'H':
	    cmd_h();
	    break;
	case 'l':
	case 'L':
	    valve_params_load();
	    break;
	case 's':
	case 'S':
	    valve_params_store();
	    break;
	case 'p':
	case 'P':
	    cmd_p();
	    break;
	case 'm':
	case 'M':
	    cmd_m();
	    break;
	case 'c':
	case 'C':
	    cmd_c();
	    break;  //finish
	case 'g':
	case 'G':
	    coder_evt_gather();
	    break;
	case 'i':
	case 'I':
	    cmd_i();
	    break;
	case 'r':
	case 'R':
	    cmd_r();
	    break;

	default: break;
    }  //switch(key)
}
