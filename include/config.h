#ifndef CONFIG_REG_H
#define CONFIG_REG_H
/*
  config.h ---- header for config
  Copyright (C) 2016 Research Center for Nuclear Physics, Osaka University
  Author: A. Tamii, Rsearch Center for Nuclear Physics
  Version 1.00 11-APR-2016 by A. Tamii
*/

/*
Local Variables:
mode: C
tab-width: 2
End:
*/



// prototype funcitons
void config_set_debug_mode(int debug_mode);
int config_read_file(char *filename);
long config_get_l_value(char *name, int index, long default_value);
double config_get_d_value(char *name, int index, double default_value);
char* config_get_s_value(char *name, int index, char* default_value);


#endif // for ifdef CONFIG_REG_H
