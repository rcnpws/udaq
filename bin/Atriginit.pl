#!/usr/bin/perl
#
#
# triginit.pl 
#      Initialize triger condition (Interactive)
#      2003/09/20   M.Uchida
#

$udaq_path="$ENV{'HOME'}"."/udaq/bin";

print "Trigger initialize program\n";
print "Please type trigger mode\n";
print " 1. GR single\n 2. GR+another event(SSD)\n 3. GR+FPP\n 4. GR+FPP+LAS\n 5. LAS single\n 6. GR+LAS\n 7. GR+LAS+SSD\n 8. GR+LAS(2nd level)\n 9. custum\n";
print "Enter number:";
$num = <STDIN>;
chop($num);
if(( $num < 1) || ($num > 9)) {
    print "Invalid number $num!\n";
    exit(1);
}

# print "Selected number: $num\n";

trig_mode: {
    if($num == 1){
	print "Selected GR single mode\n";
	system("$udaq_path/trig/triginit_grs");
	last trig_mode;
    }
    if($num == 2){
	print "Selected GR + another event(SSD)\n";
	system("$udaq_path/trig/triginit_grc");
	last trig_mode;
    }
    if($num == 3){
	print "Selected GR + FPP\n";
	system("$udaq_path/trig/triginit_grfpp");
	last trig_mode;
    }
    if($num == 4){
	print "Selected GR + FPP + LAS\n";
	system("$udaq_path/trig/triginit_gfl");
	last trig_mode;
    }
    if($num == 5){
	print "Selected LAS single mode\n";
	system("$udaq_path/trig/triginit_gsls");
	last trig_mode;
    }
    if($num == 6){
	print "Selected GR + LAS\n";
	system("$udaq_path/trig/triginit_gl");
	last trig_mode;
    }
    if($num == 7){
	print "Selected GR + LAS + SSD\n";
	system("$udaq_path/trig/triginit_glc");
	last trig_mode;
    }
    if($num == 8){
	print "Selected GR + LAS (2nd level)\n";
	system("$udaq_path/trig/triginit_gl2");
	last trig_mode;
    }
    if($num == 9){
	print "Selected custum mode\n";
	system("$udaq_path/trig/triginit_e263");
	last trig_mode;
    }
}



