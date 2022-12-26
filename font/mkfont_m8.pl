#!/usr/bin/perl
# Create font for MP-80
#
# Copyright (c) 2011, Sasaji

my $infile;
my $outfile="prn_font_m8.dat";
my $indata="";
my %outdatas=();
my $outdata="";


$infile="font_m8.bmp";
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
my $lines = (16+16+16+16);
my $chcodes = $lines * 16;

#
# L3:0x00-0x1f FM8:0x20-0x2f P8:0x30-0x3f
#
for($line = 0; $line < $lines; $line++) {
	$chcode = $line * 16;
	for($x=0; $x<32; $x+=2) {
		print sprintf("%04X:",$chcode);
		$outdatas{$chcode}=[];
		for($y=0; $y<16; $y+=2) {
			$pos = ($lines - $line - 1) * 16 * 32 + (15 - $y) * 32 + $x + $offset;
			$val = unpack("n",substr($indata,$pos,2));
			printf(" %04X", $val);
			push(@{$outdatas{$chcode}}, $val);
		}
		print"\n";
		$chcode++;
	}
}

print "--------\n";

for($chcode = 0; $chcode < $chcodes; $chcode++) {
	print sprintf("%04X:",$chcode);
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

print "--------\n";

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
	if ($height != 1024) {
		print "height must be 1024 pixel.\n";
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
