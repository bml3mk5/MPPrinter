#!/usr/bin/perl
# Create font for MP-80
#
# Copyright (c) 2011, Sasaji

my $infile="prn_font_m8_sk.dat";
my $outfile="font_m8_sk.bmp";
my $indata="";
my %tmpdatas=();
my %outdatas=();
my $outdata="";

print "-------- ".$infile."\n";
$indata="";
if (read_file($infile, \$indata)) {
	return 1;
}
#	if (check_bmp_header(\$indata)) {
#		return 1;
#	}

my $pos;
my $val;
my $chcode;
my @chcodes;

print "--------\n";
$pos = 0;
for($chcode = 0; $chcode < 0x140; $chcode++) {
	printf "%04x",$chcode;
	$tmpdatas{$chcode}=[];
	for($x=0; $x<16; $x++) {
		$pos = $chcode * 16 + $x;
		$val = unpack("C",substr($indata,$pos,1));
		push(@{$tmpdatas{$chcode}}, $val);
		printf " %02x",$val;
	}
	print "\n";
}
print "--------\n";
for($chcode = 0; $chcode < 0x140; $chcode++) {
	printf "%04x",$chcode;
	$outdatas{$chcode} =[];
	for($y=0; $y<8; $y++) {
		$val = 0;
		for($x=0; $x<16; $x++) {
			$val |= ((($tmpdatas{$chcode}[$x] & (1 << $y)) >> $y) << (15 - $x));
		}
		push(@{$outdatas{$chcode}}, $val);
		push(@{$outdatas{$chcode}}, $val);
		printf " %04x",$val;
	}
	print "\n";
}
print "--------\n";
$outdata = "";

for($ch=0x130; $ch>=0; $ch-=16) {
	for($y=15; $y>=0; $y--) {
		for($x=0; $x<16; $x++) {
			$chcode = $x + $ch;
			$outdata .= pack("n",$outdatas{$chcode}[$y]);
		}
	}
}

my $header = "";
print_bmp_header(\$header);
$outdata = $header . $outdata;
write_file($outfile, \$outdata);

print "\nComplete.";
getc(STDIN);



sub read_file {
	my($filename,$rdata)=@_;

	my $fh;
	my $buf="";;
	my $data="";

	if (!open($fh, $filename)) {
		print "$filename: file not open.\n";
		return 1;
	}
	binmode($fh);
	while(read($fh, $buf, 1024)) {
		$data .= $buf;
	}
	close($fh);
	$$rdata .= $data;
	return 0;
}

sub write_file {
	my($filename,$rdata)=@_;

	my $fh;
	my $buf="";;
	my $data="";

	if (!open($fh, "> ".$filename)) {
		print "$filename: file not open.\n";
		return 1;
	}
	binmode($fh);
	print {$fh} $$rdata;
	close($fh);
	return 0;
}

sub print_bmp_header {
	my($rdata)=@_;

	$offset = 62;
	$width = 256;
	$height = 320;
	$datasize = $width * $height / 8;
	$filesize = $datasize + $offset;

	# file header 14 bytes
	$$rdata = "BM";			# 2
	$$rdata .= pack("V",$filesize);	# 4
	$$rdata .= pack("V", 0);	# 4
	$$rdata .= pack("V",$offset);	# 4

	# info header 40 bytes
	$$rdata .= pack("V", 40);	# 4 size
	$$rdata .= pack("V", $width);	# 4 width
	$$rdata .= pack("V", $height);	# 4 height
	$bpp = 1;
	$$rdata .= pack("v", $bpp);	# 2 bpp
	$$rdata .= pack("v", 1);	# 2 bit count

	$$rdata .= pack("V", 0);	# 4 compress
	$$rdata .= pack("V", $datasize); # 4 data size

	$$rdata .= pack("V", 2834);	# 4 dot/m w
	$$rdata .= pack("V", 2834);	# 4 dot/m h

	$$rdata .= pack("V", 0);	# 4 palette
	$$rdata .= pack("V", 0);	# 4 imp colors

	# palette 8 bytes
	$$rdata .= pack("C", 255);	# 1 palette B
	$$rdata .= pack("C", 255);	# 1 palette G
	$$rdata .= pack("C", 255);	# 1 palette R
	$$rdata .= pack("C", 0);	# 1
	$$rdata .= pack("C", 0);	# 1 palette B
	$$rdata .= pack("C", 0);	# 1 palette G
	$$rdata .= pack("C", 0);	# 1 palette R
	$$rdata .= pack("C", 0);	# 1
	
	return 0;
}
