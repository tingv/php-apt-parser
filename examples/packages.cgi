#!/usr/bin/php
<? echo "Content-Type: text/html; charset=UTF-8\n\n"; ?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <meta name="Description" content="Debian packages in my unofficial Debian repository" />
    <link rel="stylesheet" type="text/css" title="Default" href="/packages.css" />
    <title>Debian packages in my unofficial Debian repository</title>
  </head>
  <body>
    <h1>Debian packages in my unofficial Debian repository</h1>
<?
      include_once("parse-apt-files.inc");

      $debarchiver_destdir= "/home/user/public_html/debian/dists";
      $base_url= "http://example.com/~user/debian/";

?>
    <p>
      This is where I keep my Debian packages, official and unofficial ones.
    </p>
    <p>
      The repository can be accessed by:
    </p>
    <pre>
      deb <?= $base_url ?> unstable main non-free contrib
      deb-src <?= $base_url ?> unstable main non-free contrib
    </pre>
    <p>
      Of course you may need to s/unstable/testing/ or s/unstable/stable/ accordingly. Distribution names also work (i.e. sarge, etch, sid).
    </p>
    <h2>Packages in unstable</h2>
<?
      parse_and_list(
                     Array("$debarchiver_destdir/unstable/main/binary-i386/Packages.gz",
                           "$debarchiver_destdir/unstable/contrib/binary-i386/Packages.gz",
                           "$debarchiver_destdir/unstable/non-free/binary-i386/Packages.gz"),
                     Array("$debarchiver_destdir/unstable/main/source/Sources.gz",
                           "$debarchiver_destdir/unstable/contrib/source/Sources.gz",
                           "$debarchiver_destdir/unstable/non-free/source/Sources.gz"),
                           "$debarchiver_destdir/unstable/installed",
                           "$debarchiver_destdir/../build_logs");
?>
    <h2>Packages in testing</h2>
<?
      parse_and_list(
                     Array("$debarchiver_destdir/testing/main/binary-i386/Packages.gz",
                           "$debarchiver_destdir/testing/contrib/binary-i386/Packages.gz",
                           "$debarchiver_destdir/testing/non-free/binary-i386/Packages.gz"),
                     Array("$debarchiver_destdir/testing/main/source/Sources.gz",
                           "$debarchiver_destdir/testing/contrib/source/Sources.gz",
                           "$debarchiver_destdir/testing/non-free/source/Sources.gz"),
                           "$debarchiver_destdir/testing/installed",
                           "$debarchiver_destdir/../build_logs");
?>
    <h2>Packages in stable</h2>
<?
      parse_and_list(
                     Array("$debarchiver_destdir/stable/main/binary-i386/Packages.gz",
                           "$debarchiver_destdir/stable/contrib/binary-i386/Packages.gz",
                           "$debarchiver_destdir/stable/non-free/binary-i386/Packages.gz"),
                     Array("$debarchiver_destdir/stable/main/source/Sources.gz",
                           "$debarchiver_destdir/stable/contrib/source/Sources.gz",
                           "$debarchiver_destdir/stable/non-free/source/Sources.gz"),
                           "$debarchiver_destdir/stable/installed",
                           "$debarchiver_destdir/../build_logs");
?>
  </body>
</html>
