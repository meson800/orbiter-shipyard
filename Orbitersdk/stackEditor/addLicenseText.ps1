$copyrightNotice = @"
//Copyright (c) 2015 Christopher Johnstone(meson800) and Benedict Haefeli(jedidia)
//The MIT License - See ../../LICENSE for more info
"@

$hFiles = Get-ChildItem -filter *.h
$cppFiles = Get-ChildItem -filter *.cpp
$files = $hFiles + $cppFiles
ForEach ($file in $files)
{
	$content = Get-Content $file
	if ($content | Select-String -simplematch '//Copyright (c) ' )
	{
		#this file already has the header
		echo ($file.Name + " has the header, updating")
		#deleting first two lines
		$content = ($content | select -Skip 2)
		$copyrightNotice | out-file temp.txt -encoding ASCII
		$content | out-file temp.txt -append -encoding ASCII
		mv -Force temp.txt $file.Name
	}
	else
	{
		echo ($file.Name + " does not have a header, adding")
		$copyrightNotice | out-file temp.txt -encoding ASCII
		$content | out-file temp.txt -append -encoding ASCII
		mv -Force temp.txt $file.Name
	}
}