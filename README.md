# INetGet - Lightweight CLI front-end to the WinINet API #

Please visit <http://muldersoft.com/> for news and updates!


## Introduction ##

**INetGet** is a simple command-line interface to the WinINet API. In other words, it is a program for downloading files via the [*HTTP*](https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol) (Hypertext Transfer Protocol) or [*FTP*](https://en.wikipedia.org/wiki/File_Transfer_Protocol) (File Transfer Protocol) protocols. It provides full support for [*HTTPS*](https://en.wikipedia.org/wiki/HTTPS) (HTTP over TLS) as well as [*IPv6*](https://en.wikipedia.org/wiki/IPv6).

![](img/inetget/inetget.png)

This program provides a similar functionality as the well-known [*WGet*](http://www.gnu.org/software/wget/) and [*cURL*](http://curl.haxx.se/) tools. However, in contrast to those, *INetGet* is based directly on the "native" Windows Internet programming interface ([*WinINet*](https://msdn.microsoft.com/en-us/library/windows/desktop/aa385483%28v=vs.85%29.aspx)). This comes at the advantage that *INetGet* is very small and lightweight, as it uses the HTTP(S) and FTP services provided by the operating system, instead of having to implement these protocols on its own. There are **no** external dependencies (e.g. OpenSSL or GnuTLS), except for standard system libraries that are present on *every* Windows system anyway. Still, advanced features, such as HTTPS and IPv6, are supported. Furthermore, since *INetGet* is based on the Windows crypto libraries, it uses the Windows *certificate store*. This means that, in contrast the aforementioned tools, you do **not** need to provide and maintain your own certificate bundle for HTTPS. Certificate updates as well as security fixes for the underlying cryptographic routines are automatically provided, via  Windows Update. On the downside, you will have to trust Microsoft's protocol implementations. And the availability of some features depends on the Windows version.

See here for more details:  
<http://blogs.technet.com/b/askperf/archive/2007/08/21/under-the-hood-wininet.aspx>


## System Requirements ##

INetGet works on Windows Vista or any later Windows version. Windows XP should work too, but is **not** recommended these days! The 32-Bit version of INetGet runs on all 32-Bit *and* 64-Bit Windows versions, while the 64-Bit version of INetGet requires a 64-Bit Windows version. 

*IPv6* support requires Internet Explorer 7 or later. Note that all supported Windows versions (Vista or later) already meet this requirement. Windows XP requires manual update to IE7 in order to enable IPv6 support.

As far as HTTPS support is concerned, all relevant Windows versions (XP or later) support TLS 1.0 as well as the deprecated SSL 2.0 and SSL 3.0 protocols. Support for TLS 1.1 has finally been introduced in Windows 7. And support for TLS 1.2 has been introduced in Windows 8.

See here for more details:  
<http://blogs.msdn.com/b/kaushal/archive/2011/10/02/support-for-ssl-tls-protocols-on-windows.aspx>


## Command-Line Usage ##

The basic *command-line syntax* of INetGet is extremely simple:

	INetGet.exe [options] <target_address> <output_file>

### Parameters ###

The following *required* parameters must always be included:

* **`<target_address>`**  
  Specifies the target *Internet address* (URL) to be downloaded. The URL must be in the ``<scheme>://<username>:<password>@<hostname>:<port>/<path>?<query>`` format.
  The *scheme* (protocol), *hostname* and *path* must always be specified. The *username* and *password* as well as the *port* number and the *query* string are optional.
  Only the ``http``, ``https`` and ``ftp`` protocols are currently supported. The *hostname* can be specified either as a domain name or as an IP address. The standard [IPv4](https://en.wikipedia.org/wiki/Dot-decimal_notation#IPv4_address) and [IPv6](https://en.wikipedia.org/wiki/IPv6_address#Recommended_representation_as_text) notations are supported.
  If the *port* number is absent, a default port number will be assumed. This means port #21 for FTP, port #80 for HTTP and port #443 for HTTPS. The *path* may be a single `/` character.
  The special string ``-`` may be specified in order to read the target address from the [*stdin*](https://en.wikipedia.org/wiki/Standard_streams#Standard_input_.28stdin.29) stream. When reading the URL from *stdin*, INetGet assumes that the string is passed in UTF-8 encoding.

* **`<output_file>`**  
  Specifies the output file, where the downloaded file will be written to. If the given path specification is *not* [fully-qualified](https://msdn.microsoft.com/en-us/library/windows/desktop/aa365247%28v=vs.85%29.aspx#fully_qualified_vs._relative_paths), then the relative path will be resolved starting from the "current" directory.
  The given path must point to an *existing* and *writable* directory, otherwise the download fails. If the specified file already exists, the program will try to *overwrite* the existing file!
  The special file name ``-`` may be specified in order to write all received data to the [*stdout*](https://en.wikipedia.org/wiki/Standard_streams#Standard_output_.28stdout.29) stream. Also the special file name ``null`` may be specified in order to discard all data that is received.

### Options ###

The following options *may* be included, in an arbitrary order:

* **`--verb=<verb>`**  
  Specifies the HTTP [*method*](https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol#Request_methods) (alias &ldquo;verb&rdquo;) to be used in the HTTP request. This can be one of the standard methods `GET`, `POST`, `PUT`, `DELETE` or `HEAD`. By default, the `GET` method is used.

* **`--data=<data>`**  
  Append additional data to the HTTP request. The given data is expected to be in the [*application/x-www-form-urlencoded*](http://www.w3.org/TR/html401/interact/forms.html#h-17.13.4.1) format, i.e. the standard format that used by HTML forms.
  You can specify `-` as the argument in order to read the data from the [*stdin*](https://en.wikipedia.org/wiki/Standard_streams#Standard_input_.28stdin.29) stream. Note that you probably want to specify either `--verb=POST` or `--verb=PUT` too when using *this* option.

* **`--no-proxy`**  
  Instructs the WinINet API to resolve the host name *locally*, i.e. **not** use a proxy server. If this option is absent, the system's default *proxy server* settings will be used to resolve the host name.

* **`--agent=<str>`**  
  Overwrite the default [*user agent*](https://en.wikipedia.org/wiki/User_agent#User_agent_identification) string that will be sent by INetGet in the HTTP request. This string typically starts with the `Mozilla/5.0` prefix, followed by system and program information.

* **`--no-redir`**  
  Do **not** follow HTTP redirects. If this option is absent, INetGet automatically follows the address given in the response's [`Location` field](https://en.wikipedia.org/wiki/HTTP_location), when a HTTP redirect (e.g. status `302`) is received.

* **`--insecure`**  
  Do **not** cause HTTPS requests to fail, if the server's TLS/SSL certificate is invalid (e.g. already expired or wrong DN) or cannot be validated (e.g. unknown issuer). Use this with extreme care !!!

* **`--verbose`**  
  Enables verbose logging, i.e. writes additional status information to the console. This is intended for debugging purposes and does *not* normally need to be specified.

* **`--help`**  
  If this option is set, INetGet will print the "help screen" to the console and then exit immediately.

### Exit Codes ###

If the download completed successfully, INetGet returns a *zero* (`0`) exit code. Otherwise, if something went wrong (connection to server failed, file not found, etc), it return a *non-zero* (`1`) error code.

Note that only HTTP [*status codes*](https://en.wikipedia.org/wiki/List_of_HTTP_status_codes) in the `2xx` range will be considered a successful transfer. If the server returns a different status code, e.g. in the `4xx` or `5xx` range, the transfer has failed.

### Examples ###

Here are some basic examples that show the command-line usage of INetGet:

* Download a simple HTML document:  
 `INetGet.exe http://www.warr.org/buckethead.html output.html`

* Send request with query string:  
  `INetGet.exe http://www.google.de/search?q=drunkship+of+lanterns output.html`

* Send request with "POST" method and form data included:  
  `INetGet.exe --verb=POST --data="foo=hello&bar=world" http://muldersoft.sourceforge.net/test.php output.txt`

* Request with user login and port number specified:  
  `INetGet.exe http://alice:jelly22fi$h@localhost:8080/secret.html output.html`

* Read the URL from *stdin* and write the response to *stout*:  
  `echo http://www.warr.org/buckethead.html | INetGet.exe - - | findstr Sacrifist`


## License ##

**INetGet is Copyright &copy; 2015 LoRd_MuldeR <<MuldeR2@GMX.de>>. Some rights reserved.**  
**This software is released under the GNU General Public License (&ldquo;GPL&rdquo;), version 2.**

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

**See also:**  
<http://www.gnu.org/licenses/gpl-2.0-standalone.html>

&nbsp;

**e.o.f.**
