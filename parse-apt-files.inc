<?php
    //////////////////////////////////////////////////////////////////
    // PHP Apt-file parser, Version 1.3
    //
    // The following PHP functions parse Debian APT-repository files
    // (Packages.gz, Sources.gz) and generate an XHTML summary of them.
    //
    // Written by Jarno Elonen in 2003, 2004
    // Written by Daniel Leidert in 2005, 2006
    // Written by Damian Viano in 2006
    //
    //////////////////////////////////////////////////////////////////

    // Parse Packages.gz or Sources.gz file and return an array of format
    // Array( "<pkg-name>", Array( "<Field-name>" => "<value>", ... ))
    // Note that the result may contain several versions of the same package
    function parse_packages( $filename )
    {
        $lines = (strpos($filename, ".gz") !== False)
          ? gzfile($filename) : file($filename);

        $i = -1;
        $curtag = false;
        $newtag = true;
        $value = "";
        $res = Array();
        foreach( $lines as $line )
        {
            $matches = Array();
            if ( preg_match("/^([^ ]*):( )?(.*)$/", $line, $matches) )
            {
                if ( $curtag !== false && $i >= 0 )
                    $res[$i][1][$curtag] = $value;

                $curtag = $matches[1];
                $value = $matches[3];

                if ( $curtag == "Package" )
                    $res[++$i] = Array(0 => $value, 1 => Array());
                $newtag = true;
            }
            else
            {
                if ( trim($line) !== "." )
                {
                    if ( $newtag || substr($line, 0, 2) == "  " )
                        $value .= "\n";
                    else
                        $value .= " ";
                    $value .= trim($line);
                }
                else $value .= "\n\n";
                $newtag = false;
            }
        }
        if ( $curtag !== false && $i >= 0 )
            $res[$i][1][$curtag] = $value;

        return $res;
    }

    // Convert given byte-size to more human readable format
    function human_format_size( $bytes )
    {
        $formats = Array("%d Bytes", "%.1f KB", "%.1f MB", "%.1f GB",
                         "%.1f TB");
        $logsize = min((int)(log($bytes)/log(1024)), count($formats)-1);
        return sprintf($formats[$logsize], $bytes/pow(1024, $logsize));
    }

    // Extract URLs from text, make them links and output
    // the whole thing as HTML
    function link_aware_htmlize( $txt )
    {
        $res = "";
        $linkpattern = "!([a-z]+://[^ \n    \)$]+)!is";
        $frags = preg_split($linkpattern, $txt, -1, PREG_SPLIT_DELIM_CAPTURE);
        foreach( $frags as $f )
        {
            if ( preg_match($linkpattern, $f) )
                $res .= "<a href=\"" . htmlentities($f) . "\">" .
                        htmlentities($f) . "</a>";
            else
                $res .= htmlentities($f);
        }
        return str_replace("\n", "<br/>\n", $res);
    }

        function parse_section( $section )
        {
                $res = "<span class=\"section\">";
                if ( preg_match("!(contrib/[a-z]+)!is", $section) )
                    $res .= ", <span class=\"contrib\">contrib</span>";
                else if ( preg_match("!(non-free/[a-z]+)!is", $section) )
                    $res .= ", <span class=\"nonfree\">non-free</span>";
                else
                    $res .= ", <span class=\"main\">main</span>";
                return $res . "</span>";
        }

    // Parse given files and write out a nice summary in HTML
    function parse_and_list( $packagesgzfiles, $sourcesgzfile,
                             $shared_changesdir = False,
                             $shared_buildlogsdir = False )
    {
        global $base_url;
        if ( !is_array($sourcesgzfile) )
            $sourcesgzfile = Array($sourcesgzfile);
        
        // Rebuild sources array
        $sources = Array();
        foreach( $sourcesgzfile as $f )
        {
            $sources_temp = parse_packages($f);
            while( list($i, $p) = each($sources_temp) )
                    $sources[$p[0]] = $p[1];
        }

        if ( !is_array($packagesgzfiles) )
            $packagesgzfiles = Array($packagesgzfiles);

        // Merge different architectures into one $packages array
        $packages = Array();
        foreach( $packagesgzfiles as $f )
        {
            $pkgs = parse_packages($f);
            while( list($i, $p) = each($pkgs) )
            {
                $name_and_ver = $p[0] . "-" . $p[1]["Version"];
                if ( !isset($packages[$name_and_ver]) )
                    $packages[$name_and_ver] = Array();
                $packages[$name_and_ver][$p[1]["Architecture"]] = $p;
            }
        }

        // Walk through the packages and print out the info
        while( list($name_and_ver, $archs) = each($packages) )
        {
            print "<div class=\"package\">\n";

            // $first = false;
            // use the first arch for description etc.
            ksort($archs);
            list($pkg, $fields) = reset($archs);

            $srcfields = false;
            if ( isset($fields["Source"]) &&
                 isset($sources[$fields["Source"]]) )
                    $srcfields = $sources[$fields["Source"]];
            else if ( isset($sources[$pkg]) )
                $srcfields = $sources[$pkg];

            print "<span class=\"name\">" .
                  htmlentities($fields["Package"]) . "</span> " .
                  "<span class=\"version\">(" .
                  htmlentities($fields["Version"]) .
                  parse_section($fields["Section"]) . ")</span>" .
                  "\n";

            if ( strlen(trim($fields["Description"])) )
                print " - <span class=\"short_description\">" .
                      htmlentities(preg_replace("/[\n].*/", "",
                        $fields["Description"]));
            print "</span><br/>\n";

            $archlinks = Array();
            while( list($arch_name, list(, $arch_fields)) = each($archs) )
                $archlinks[] = "<a href=\"" .
                    htmlentities($base_url) .
                    htmlentities($arch_fields["Filename"]) . "\">" .
                    htmlentities($arch_name) . " (" .
                    human_format_size($arch_fields["Size"]) . ")</a>";
            print "Binary for arch <span class=\"archs\">" .
                  join(", ", $archlinks) . "</span>\n";


            
            // Figure out the most likely location for "changes" file.
            $changesdir = False;
            if ( $shared_changesdir !== False )
                $changesdir = $shared_changesdir;
            else if ( isset($srcfields["Directory"]) )
                $changesdir = $srcfields["Directory"];
            else
            {
                $changesdir = pathinfo($some_bin);
                $changesdir = $changesdir["dirname"];
            }

            $buildlogsdir = False;
            if ( $shared_buildlogsdir !== False )
                $buildlogsdir = $shared_buildlogsdir;
            else if ( isset($srcfields["Directory"]) )
                $buildlogsdir = $srcfields["Directory"];
            else
            {
                $buildlogsdir = pathinfo($some_bin);
                $buildlogsdir = $buildlogsdir["dirname"];
            }

            // If sources are available, print out location.
            if ( isset($srcfields["Directory"]) )
            {
                print "| <a href=\"" .
                    htmlentities($base_url) .
                    htmlentities($srcfields["Directory"]) .
                    "\">Source dir</a>" . "\n";

            }
  
            // Try to make a changes file link. This adds quite a bit
            // of disk access so if your server is very busy, you might
            // want to comment this out.
            if ( $changesdir !== False && $dhandle = opendir($changesdir) )
            {
                $chfile = False;
                while ( false !== ($file = readdir($dhandle)) )
                {
                    // Packages with multiple binaries or a source-package
                    // name != binary-package name normally have a Source-field
                    // in Packages(.{gz,bz2}).
                    if ( isset($fields["Source"]) )
                    {
                        if ( strpos($file, $fields["Source"] . "_" .
                             preg_replace("/^[0-9]+\:/", "",
                             $fields["Version"])) !== False &&
                             strpos($file, ".changes") !== False )
                            $chfile = $file;
                    }
                    // If there is no Source-field, just try to use the
                    // Packages-field in Sources(.{gz,bz2}).
                    else if ( isset($srcfields["Package"]) )
                    {
                        if ( strpos($file, $srcfields["Package"] . "_" .
                             preg_replace("/^[0-9]+\:/", "",
                             $fields["Version"])) !== False &&
                             strpos($file, ".changes") !== False )
                            $chfile = $file;
                    }
                    // And at least simply try the Package-field from
                    // Packages(.{gz,bz2}) for those packages, that do not have
                    // a Sources(.{gz,bz2}) entry.
                    else
                    {
                        if ( strpos($file, $fields["Package"] . "_" .
                             preg_replace("/^[0-9]+\:/", "",
                             $fields["Version"])) !== False &&
                             strpos($file, ".changes") !== False )
                            $chfile = $file;
                    }
                    if ( $chfile !== False )
                    {
                        print "| <a href=\"" .
                            htmlentities($base_url) .
                            htmlentities(preg_replace("!.*" . 
                                                        preg_replace(
                                                           "!.*(/[^/]+/?)$!",
                                                           "$1", 
                                                           $base_url) . 
                                                        "(.*)!", "$1",
                                                        $changesdir) . 
                                          "/" . $chfile) .
                            "\">Changes</a>" . "\n";
                        break;
                    }
                }
                closedir($dhandle);
            }

            // Try to make a build-log file link. This adds quite a bit
            // of disk access so if your server is very busy, you might
            // want to comment this out.
            if ( $buildlogsdir !== False && $dhandle = opendir($buildlogsdir) )
            {
                $blfile = False;
                while ( false !== ($file = readdir($dhandle)) )
                {
                    // Packages with multiple binaries or a source-package
                    // name != binary-package name normally have a Source-field
                    // in Packages(.{gz,bz2}).
                    if ( isset($fields["Source"]) )
                    {
                        if ( strpos($file, $fields["Source"] . "_" .
                             preg_replace("/^[0-9]+\:/", "",
                             $fields["Version"])) !== False &&
                             strpos($file, ".changes") !== False )
                    }
                    // If there is no Source-field, just try to use the
                    // Packages-field in Sources(.{gz,bz2}).
                    else if ( isset($srcfields["Package"]) )
                    {
                        if ( strpos($file, $srcfields["Package"] . "_" .
                             preg_replace("/^[0-9]+\:/", "",
                             $fields["Version"])) !== False &&
                             strpos($file, ".changes") !== False )
                    }
                    // And at least simply try the Package-field from
                    // Packages(.{gz,bz2}) for those packages, that do not have
                    // a Sources(.{gz,bz2}) entry.
                    else
                    {
                        if ( strpos($file, $fields["Package"] . "_" .
                             preg_replace("/^[0-9]+\:/", "",
                             $fields["Version"])) !== False &&
                             strpos($file, ".changes") !== False )
                            $blfile = $file;
                    }
                    if ( $blfile !== False )
                    {
                        print "| <a href=\"" .
                            htmlentities($base_url) .
                            htmlentities(preg_replace("!.*" . 
                                                        preg_replace(
                                                           "!.*(/[^/]+/?)$!",
                                                           "$1", 
                                                           $base_url) . 
                                                        "(.*)!", "$1",
                                                        $buildlogsdir) . 
                                          "/" . $blfile) .
                            "\">Build-Log</a>" . "\n";
                        break;
                    }
                }
                closedir($dhandle);
            }
            
            print "<br/><br/>\n" .
                  "<blockquote><p class=\"description\">\n" .
                  link_aware_htmlize(
                    preg_replace("/^[^\n]*\n/", "", $fields["Description"])) .
                  "\n</p>\n" .
                  "</blockquote>\n</div>\n";
        }
    }
?>
