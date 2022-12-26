#!/usr/bin/perl
# Create font for MP-1020
#
# Copyright (c) 2011, Sasaji

my $infile;
my $outfile="prn_font_l3.dat";
my $indata="";
my %outdatas=();
my $outdata="";


	$infile="font_l3.bmp";
	print "-------- ".$infile."\n";
	$indata="";
	if (read_file($infile, \$indata)) {
		return 1;
	}
	if (check_bmp_header(\$indata)) {
		return 1;
	}

my $pos;
my $val;
my $chcode;
my @chcodes;

@chcodes=(0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0, 0x100, 0x110, 0x120, 0x130);
for($line = 0; $line < 20; $line++) {
	$chcode = $chcodes[$line];
	for($x=0; $x<32; $x+=2) {
		print sprintf("%02X:",$chcode);
		$outdatas{$chcode}=[];
		for($y=0; $y<16; $y+=2) {
			$pos = (19 - $line) * 16 * 32 + (15 - $y) * 32 + $x + $offset;
			$val = unpack("n",substr($indata,$pos,2));
			printf(" %04X", $val);
			push(@{$outdatas{$chcode}}, $val);
		}
		print"\n";
		$chcode++;
	}
}

for($chcode = 0; $chcode < 0x140; $chcode++) {
	print sprintf("%02X:",$chcode);
	for($x = 0; $x < 16; $x++) {
		$val = 0;
		for($y = 0; $y < 8; $y++) {
			my $fi = (0x8000 >> $x);
			my $fo = ($outdatas{$chcode}[$y] & $fi) ? (1 << $y) : 0;
			$val += $fo;
		}
		$outdata .= pack("C", $val);
		printf(" %02X", $val);
	}
	print"\n";
}

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

sub check_bmp_header {
	my($rdata)=@_;

	# file header 14 bytes
	if (substr($$rdata,0,2) ne "BM") {
		print "This is not BMP format.\n";
		return 1;
	}
	$offset = unpack("V",substr($$rdata,0x0a,4));
	print "offset:".$offset."\n";

	# info header 
	my $infosize = unpack("V",substr($$rdata,0x0e,4));
	if ($infosize != 40) {
		print "Windows BMP format only.\n";
		return 1;
	}
	my $width = unpack("V",substr($$rdata,0x12,4));
	if ($width != 256) {
		print "width must be 256 pixel.\n";
		return 1;
	}
	my $height = unpack("V",substr($$rdata,0x16,4));
	if ($height != 320) {
		print "height must be 320 pixel.\n";
		return 1;
	}
	my $bpp = unpack("v",substr($$rdata,0x1c,2));
	if ($bpp != 1) {
		print "Supported data is only 1bit per pixel(B/W data).\n";
		return 1;
	}
	my $compress = unpack("V",substr($$rdata,0x1e,4));
	if ($compress != 0) {
		print "Supported data is only no compression.\n";
		return 1;
	}

	return 0;
}
