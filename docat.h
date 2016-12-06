class DoCatObject{
	// Name, title
	MString name;
	MString title;

	// Options
	int notfound;

public:

	DoCatObject(){
		notfound = 0;
	}

	VString GetName(){
		return name;
	}

	VString GetTitle(){
		return title;
	}

	int GetNotFound(){
		return notfound;
	}

	void SetName(VString v){
		name = v;
	}

	void SetTitle(VString v){
		title = v;
	}

	void SetNotFound(){
		notfound = 1;
	}

};

bool DoCatObjectFind(DoCatObject &obj, VString name){
	return name == obj.GetName();
}

class DoCatObjects{
	OList<DoCatObject> objects;

public:
	bool IsObj(VString name){
		return objects.Search(DoCatObjectFind, name) != 0;
	}

	DoCatObject* GetObj(VString name){
		return objects.Search(DoCatObjectFind, name);
	}

	DoCatObject* AddObj(VString name){
		DoCatObject *obj = objects.NewE();
		obj->SetName(name);
		
		return obj;
	}
};


ConfLineOptions config;
DoCatObjects objects;

#define DOCAT_ERROR_NAMELINE() "In '", name, "', line ", itos(DoCatLineCount(data, ln) + 1), "\r\n"

int DoCatLineCount(unsigned char *ln, unsigned char *to){
	int linecount = 0;

	while(ln < to){
		if(*ln =='\n')
			linecount ++;
		ln ++;
	}

	return linecount;
}

TString DoCatCode(unsigned char *ln, unsigned char *to){
	LString res;
	unsigned char *lln = ln;

	while(ln < to){
		if(*ln == '<'){
			// Flush
			res + VString(lln, ln - lln);
			lln = ln + 1;

			res + "&lt;";
		}			
		ln ++;
	}

	// Flush
	res + VString(lln, ln - lln);
	lln = ln;
	
	return (TString)res;
}

TString DocatContentId(unsigned char *ln, unsigned char *to){
	LString res;
	unsigned char *lln = ln;

	while(ln < to){
		if(*ln == ' '){
			// Flush
			res + VString(lln, ln - lln);
			lln = ln + 1;

			res + "_";
		}			
		ln ++;
	}

	// Flush
	res + VString(lln, ln - lln);
	lln = ln;
	
	return (TString)res;

}


//// Options //

// Source & destination dir, extension
VString src, dst, ext;

// Html title, begin, end
MString html_title, html_begin, html_end;

// Options. No CrLf, No Exists, No Skip, Make Directories.
int opt_crlf, opt_noex, opt_noskip, opt_makedir;

class DocatOptions{
public:

	DocatOptions(){	}

	int Load(){
		// Source & destination dir
		src = config.GetOption("Src");
		dst = config.GetOption("Dst");
		ext = config.GetOption("Ext");

		// Add html
		//html_title = config.GetOption("HtmlTitle");
		//html_begin = config.GetOption("HtmlBegin");
		//html_end = config.GetOption("HtmlEnd");
		if(!LoadHtml("HtmlTitle", html_title) || !LoadHtml("HtmlBegin", html_begin) || !LoadHtml("HtmlEnd", html_end))
			return 0;

		// Options
		opt_crlf = config.GetOption("NoCrLf").compareu("Off");
		opt_noex = config.GetOption("NoExist").compareu("On");
		opt_noskip = config.GetOption("NoSkip").compareu("On");
		opt_makedir = config.GetOption("MakeDir").compareu("On");

		//int linecount = 0;

		if(!ext)
			ext = ".txt";

		return 1;
	}

	int LoadHtml(VString name, MString &data){
		SString path;

		if(!config.GetOption(name))
			return -1;

		path.Add(src, "/", config.GetOption(name));

		if(!IsFile(path)){
			print(LString() + "Error! " + name + " not found in: " + path + "\r\n");
			return 0;
		}

		data = LoadFile(path);

		return 1;
	}

} options;


// Result //
// <html>
// <head> + title + HtmlTitle + </head>
// <body> + HtmlBegin + fbody + content + body + HtmlEnd </body>
// </html>

class DocatResult{
public:
	// Html title
	LString0 title;

	// Content menu
	LString0 content;

	// First body & body
	LString0 fbody, body;

	// Output
	LString0 out;

	// Hash
	MString hash;

	void Hash(){
		MHash mh(MHASHT_MD5);
		mh.Add(title);
		mh.Add(content);
		mh.Add(fbody);
		mh.Add(body);
		mh.Add(html_title);
		mh.Add(html_begin);
		mh.Add(html_end);
		mh.Finish();

		hash = mh.GetH();

		return ;
	}

	VString Out(){
		out.Clean();

		out + "<!DOCTYPE html>\r\n<html>\r\n<head>\r\n" + title + html_title + "\r\n</head>\r\n<body>\r\n" + html_begin + fbody;
			
		if(content)
			out + "<table border=0 cellpadding=0 cellspacing=3><tr><td align=center><h3>Contents:</h3></td></tr>" + content + "</table>\r\n";
	
		out + body + html_end + "</body>\r\n</html>\r\n";

		return out;
	}

};

int DoCatIndex(VString name){
	if(objects.IsObj(name))
		return -1;

	DoCatObject *obj = objects.AddObj(name);

	// Path, data, result, head result
	DocatResult result;
	SString path;
	MString data;
	//LString hres, res;
	LString0 &res = result.body;

	// Contents
	//LString cres;
	unsigned int clevel[5] = {0, 0, 0, 0, 0};

	// Load source
	path.Add(src, "/", name, ext);
	
	if(!IsFile(path)){
		if(opt_noex){
			obj->SetNotFound();

			print(LString() + "Warning! Path not found: " + path + "\r\n");
			return -1;
		}

		print(LString() + "Error! Path not found: " + path + "\r\n");
		return 0;
	}

	data = LoadFile(path);

	// Analysis head
	unsigned char *ln = data, *lln = ln, *to = data.endu(), *pln;
	if(rtms(to, (unsigned char*)"\r\n\r\n", 4, ln)){
		obj->SetTitle(VString(lln, ln - lln));		

		result.title + "<title>" + VString(lln, ln - lln) + "</title>\r\n";
		lln = ln += 4;
	} else{
		print(LString() + "Error! Title not found in: " + name + "\r\n");
		return 0;
	}

	// Add title
	res + "<h1>" + obj->GetTitle() + "</h1><hr>\r\n";

	// Analysis body
	while(ln < to){
		// == H ==
		if(*ln == '=' && ( *(ln - 1) == '\n' || *(ln - 1) == '\r' ) ){
			pln = ln;
			//if(*(pln - 1) == '\n')
			//	pln --;

			//if(*(pln - 1) == '\r')
			//	pln --;
			
			// Flush
			res + VString(lln, pln - lln);
			lln = pln = ln;

			unsigned int cnt = 0, ecnt = 0;
			while(ln < to && *ln == '='){
				cnt ++;
				ln ++;
			}
			lln = ln;

			if(ln >= to){
				print("Error! Unexpectedly EOF after '='.", DOCAT_ERROR_NAMELINE());
				return 0;
			}

			while(ln < to && *ln != '=')
				ln ++;

			VString text(lln, ln - lln);
			text = dspacev(text);

			lln = ln;
			while(ln < to && *ln == '='){
				ecnt ++;
				ln ++;
			}

			//if(ln >= to){
			//	print("Error! Unexpectedly EOF after '= h ='. ", DOCAT_ERROR_NAMELINE());
			//	return 0;
			//}

			if(cnt != ecnt){
				print("Error! Count of first '=' != second '='. ", DOCAT_ERROR_NAMELINE());
				return 0;
			}

			TString cid = DocatContentId(text, text.endu());

			// Add to content
			if(cnt < 6){
				int p = 0;

				cnt --;

				if(!result.fbody){
					result.fbody = res;
					res.Clean();
				}

				// Id
				result.content + "<tr><td>";
				result.content + "<a href='#" + cid + "'>";

				clevel[cnt] ++;
				for(unsigned int i = cnt + 1; i < 5; i ++)
					clevel[i] = 0;

				for(unsigned int i = 0; i <= cnt; i ++){
					if(!clevel[i])
						continue;

					if(p){
						result.content + ".";
						p = 0;
					}

					p = 1;
					result.content + clevel[i];
				}

				result.content + " " + text + "</a></td></tr>";
			}

			// Switch
			if(cnt < 5){
				res + "<h" + (cnt + 1) + "><span id='" + cid + "'>" + text + "</span></h" + (cnt + 1) + ">";
				if(cnt < 2)
					res + "<hr>";
			}
			else 
				res + VString(lln, ln - lln) + text + VString(lln, ln - lln);
			
			if(ln < to && *ln == '\r')
				ln ++;

			if(ln < to && *ln == '\n')
				ln ++;

			lln = ln;
			continue;
		}

		// [[ link ]]
		if(*ln == '[' && (ln + 1) < to && *(ln + 1) == '['){
			// Flush
			res + VString(lln, ln - lln);
			lln = pln = ln += 2;

			// Find end ]]
			while(ln < to){
				if(*ln == ']' && (ln + 1) < to && *(ln + 1) == ']'){
					break;
				}
				if(*ln == '|')
					pln = ln;
				ln ++;
			}

			if(ln >= to){
				print("Error! Close ]] not found. ", DOCAT_ERROR_NAMELINE());
				return 0;
			}

			VString link, desc, alt;
			SString tmp;
			int notfound = 0;

			if(pln == lln){
				link = desc = VString(lln, ln - lln);
			} else{
				link = VString(lln, pln - lln);
				desc = VString(pln + 1, ln - pln - 1);
			}

			if(link.incompare("wikipedia:")){
				if(link == desc)
					desc = desc.str(10);

				link = tmp.Add("https://en.wikipedia.org/wiki/", link.str(10));
			}
			else if(link.incompare("http://") || link.incompare("https://")){
				// no action
			}
			else{
				DoCatObject *lobj;

				if(!(lobj = objects.GetObj(link))){
					if(!DoCatIndex(link))
						return 0;

					lobj = objects.GetObj(link);
				}

				alt = lobj->GetTitle();
				notfound = lobj->GetNotFound();

				link = tmp.Add(link, ".html");
			}

			res + "<a href='" + link + "'";
			if(alt)
				res + " title='" + alt + "'";
			res + ">" + desc;
			if(notfound)
				res + "[BAD LINK]";
			res + "</a>";

			lln = ln += 2;
			continue;
		} // end [[link]]

		// <tags>
		if(*ln == '<'// && 
			//(ln + 6 < to && cmp(ln, (unsigned char*)"<code>", 6))
		){
			// Flush
			res + VString(lln, ln - lln);
			lln = pln = ln += 1;

			// Find end
			while(ln < to && ( *ln >= 'a' && *ln <= 'z' || *ln >= 'A' && *ln <= 'Z'))
				ln ++;

			if(ln >= to){
				print("Error! Open <tag> fail. ", DOCAT_ERROR_NAMELINE());
				return 0;
			}

			if(*ln != '>'){
				lln --;
				continue;
			}

			// Tag name
			VString tag(lln, ln - lln);
			lln = ln + 1;

			// Find end tag
			while(ln <= to){
				if(ln >= to){
					print("Error! Close </", tag, "> not found. ", DOCAT_ERROR_NAMELINE());
					return 0;
				}

				if(*ln == '<' && ln + tag.sz + 3 <= to && *(ln + 1) == '/' && cmp(ln + 2, tag, tag))
					break;

				ln ++;
			}

			if(tag == "nowiki"){
				res + VString(lln, ln - lln);
			} else if(tag == "code" || tag == "pre"){
				res + "<pre>" + DoCatCode(lln, ln) + "</pre>";
			} else if(tag == "text"){
				res + DoCatCode(lln, ln);
			} else{
				res + "<" + tag + ">" + VString(lln, ln - lln) + "</" + tag + ">";
			}
			lln = ln += 3 + tag.sz;

			continue;
		}

		// CR + LF
		if(*ln == '\r'){
			// Flush
			res + VString(lln, ln - lln + opt_crlf);
			lln = ln += 1;
			continue;
		}

		if(*ln == '\n'){
			// Flush
			res + VString(lln, ln - lln + opt_crlf);
			lln = ln += 1;

			res + "<br>";
			continue;
		}

		ln ++;
	}

	// Flush
	res + VString(lln, ln - lln);

	// Add contents
	//if(!hres)
	//	hres + res;
	//else if(cres){
	//	hres + "<table border=1 cellpadding=0 cellspacing=0><tr><td align=center>Contents:</td></tr>" + cres + "</table>\r\n" + res;
	//}

	// Output
	//VString sout = hres;
	//LString out;
	MTime mt;

	// If not modified
	path.Add(dst, "/", name, ".html");
	MString cdata = LoadFile(path);
	VString chash = PartLineDouble(cdata, "md5='", "'");

	//if(chash == md5h(sout))
	//	return 1;

	// Base head
	//out + "<html>\r\n<head>\r\n";

	//out + hres;

	// Add comment
	//out + "<p><font color=gray>" + "[Generation date: " + mt.date("d.m.y H:i:s") + "]</font>";

	result.Hash();
	
	if(chash == result.hash && !opt_noskip)
		return 1;

	// Head & hash
	result.title + "<meta name='Docat Wiki Generator' ver='" + PROJECTVER[0].ver + "' " \
	"warning='Do Not Write To This File, All Data Well Be Lost!'" " " \
	"md5='" + result.hash + "' data='" + mt.date("d.m.y H:i:s") + "' />\r\n";

	VString out = result.Out();

	// Save
	if(!SaveFile(path, out)){
		print("Error! File not saved: ", path, "\r\n");
		return 0;
	}

	return 1;
}

int DoCatMakeDir(VString path){
	unsigned char *ln = path.data + dst.size() + 1, *to = path.endu(), *p = to;

	return MkDir(VString(path.data, p - path.data));


	while(p >= ln){
		if(p == to || *p =='/' || *p == '\\')
			if(MkDir(VString(path.data, p - path.data)))
				break;
		p --;
	}

	while(p <= to){
		if(p == to || *p =='/' || *p == '\\')
			if(!MkDir(VString(path.data, p - path.data)))
				return 0;
		p ++;
	}

	return 1;
}

int DoCatCopy(VString line){
	VString from, to;
	SString path;

	from = PartLineST(line, to);

	print("Copy file. From: ", src, "/", from, ", to: ", dst, "/", to, ".\r\n");

	if(!from || !to){
		print("Error! Copy ", line, ". Bad parameters.\r\n");
		return 0;
	}

	// Load
	path.Add(src, "/", from);

	if(!IsFile(path)){
		print("Error! File ", path, " not found.\r\n");
		return 0;
	}

	MString data = LoadFile(path);

	// Save
	path.Add(dst, "/", to);

	if(!SaveFile(path, data)){
		if(opt_makedir){
			ILink ilink(path);
			DoCatMakeDir(ilink.GetProtoDomainPath());
		}

		if(!SaveFile(path, data)){
			print("Error! Save file ", path, " error.\r\n");
			return 0;
		}
	}

	return 1;
}

int DoCat(){
	if(!IsFile("docat.conf") || !config.LoadFile("docat.conf")){
		print("Error! File docat.conf not found in this path.\r\n");
		return 0;
	}

	if(!options.Load())
		return 0;

	ConfLineOption *opt = 0;
	
	while(opt = config.FindOption("index", opt)){
		if(opt->val)
			if(!DoCatIndex(opt->val))
				return 0;			
	}

	// Copy
	opt = 0;

	while(opt = config.FindOption("copy", opt)){
		if(opt->val)
			if(!DoCatCopy(opt->val))
				return 0;			
	}


	print("DoCat OK.\r\n");

	return 1;
}