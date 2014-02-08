/*
 *  objexp.h - Obsolete regular expression implementation.
 *  ========
 *
 *  Created by John Holdsworth on 01/04/2009.
 *  Copyright 2009 © John Holdsworth. All Rights Reserved.
 *
 *  $Id: //depot/ObjCpp/objstr.h#89 $
 *  $DateTime: 2012/11/18 19:11:51 $
 *
 *  C++ classes to wrap up XCode classes for operator overload of
 *  useful operations such as access to NSArrays and NSDictionary
 *  by subscript or NSString operators such as + for concatenation.
 *
 *  This works as the Apple Objective-C compiler supports source
 *  which mixes C++ with objective C. To enable this: for each
 *  source file which will include/import this header file, select
 *  it in Xcode and open it's "Info". To enable mixed compilation,
 *  for the file's "File Type" select: "sourcecode.cpp.objcpp".
 *
 *  For bugs or ommisions please email objcpp@johnholdsworth.com
 *
 *  Home page for updates and docs: http://objcpp.johnholdsworth.com
 *
 *  If you find it useful please send a donation via paypal to account
 *  objcpp@johnholdsworth.com. Thanks.
 *
 *  License
 *  =======
 *
 *  You may make commercial use of this source in applications without
 *  charge but not sell it as source nor can you remove this notice from
 *  this source if you redistribute. You can make any changes you like
 *  to this code before redistribution but you must annotate them below.
 *
 *  For further details http://objcpp.johnholdsworth.com/license.html
 *
 *  THIS CODE IS PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND EITHER
 *  EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING
 *  WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MODIFIES AND/OR CONVEYS
 *  THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, INCLUDING
 *  ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT
 *  OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED
 *  TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED
 *  BY YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH
 *  ANY OTHER PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN
 *  ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 *
 */

#ifndef _objexp_h_
#define _objexp_h_

extern "C" {
    #include <regex.h>
}

// "ENHANCED" regexps available to deploy on OSX 10.8/iOS 6.0+
#ifdef REG_ENHANCED
#define OO_REG_FLAGS REG_EXTENDED|REG_ENHANCED
#else
#define OO_REG_FLAGS REG_EXTENDED
#endif

/**
 A class to represent a patten matching operations against a string.

 Usage:
 <pre>
 OOStringArray words = OOPattern( @"\\w+" ).match();
 </pre>
 */

class OOPattern {
	OOString pat;
    oo_inline OOStringArray matchGroups( const char *input, regmatch_t *matches ) const {
        OOStringArray groups;
        for ( int i=0 ; i<=regex->re_nsub ; i++ )
            groups[i] = matches[i].rm_so == -1 ? OOString( OONull ) :
            OOString( input+matches[i].rm_so, (NSUInteger)(matches[i].rm_eo-matches[i].rm_so) );
        return groups;
    }
public:
	regex_t *regex;

	oo_inline OOPattern() {}
	oo_inline OOPattern( cOOString patin, int flags = OO_REG_FLAGS ) {
        init( patin, flags );
    }
	oo_inline void init( cOOString patin, int flags );

	oo_inline int exec( const char *input, int matchmax = 0, regmatch_t matches[] = NULL, int eflags = 0 ) const {
#ifndef DEBUG
        if ( !input || !regex )
            return 0;
#endif
		int error = regexec( regex, input, matchmax, matches, eflags );
		if ( error && error != REG_NOMATCH ) {
			char errbuff[1024];
			regerror( error, regex, errbuff, sizeof errbuff );
			OOWarn( @"OOPattern::exec() Regex match error: %s, in pattern'%@'", errbuff, *pat );
		}
		return error == 0;
	}
    oo_inline NSRange range( cOOString str ) {
        NSRange range = {NSNotFound, 0};
        const char *input = str;
        regmatch_t match;

        if ( exec( input, 1, &match ) ) {
            range.location = [OOString( input, (int)match.rm_so ) length];
            range.length = [OOString( input+match.rm_so, (int)(match.rm_eo-match.rm_so) ) length];
        }

        return range;
    }
	oo_inline int find( const char *input, int matchmax, regmatch_t matches[] ) const {
		int nmatches = 0;
		for ( regoff_t pos = 0 ; exec( input+pos, matchmax-nmatches, matches+nmatches ) ; nmatches++ ) {
			matches[nmatches].rm_so += pos;
			pos = matches[nmatches].rm_eo += pos;
		}
		return nmatches;
	}

	oo_inline OOStringArray split( cOOString str ) const {
		OOPoolIfRequired;
		const char *input = str;
		OOStringArray out;

		regmatch_t matches[100];
		for ( ; exec( input, sizeof matches/sizeof matches[0], matches ) ; input += matches[0].rm_eo )
			out += OOString( input, (NSUInteger)matches[0].rm_so );

		out += OOString( input );
		return out;
	}

	oo_inline OOStringArray matchAll( cOOString str ) const {
		OOPoolIfRequired;
		const char *input = str;
		OOStringArray out;

		regmatch_t matches[100];
        for ( ; input && exec( input, sizeof matches/sizeof matches[0], matches ) ; input += matches[0].rm_eo )
            out += OOString( input+matches[0].rm_so, (NSUInteger)(matches[0].rm_eo-matches[0].rm_so) );

		return out;
	}

	oo_inline OOStringArray match( cOOString str ) const {
		OOPoolIfRequired;
		const char *input = str;
		OOStringArray out;

        int outerGroups[100], *oend = outerGroups;
        for ( int ptr = 0, group = 0, level = 0 ; regex->re_nsub && ptr < [pat length] &&
             oend < outerGroups+sizeof outerGroups/sizeof outerGroups[0] ; ptr++ )
            switch( pat[ptr] ) {
                case '(':
                    if ( ++group && ++level == 1 )
                        *oend++ = group;
                    break;
                case ')': --level; break;
                case '\\': ptr++; break;
            }

        if ( oend == outerGroups )
            return matchAll( str );

		regmatch_t matches[100];
        for ( ; input && exec( input, sizeof matches/sizeof matches[0], matches ) ; input += matches[0].rm_eo )
            for ( int *optr = outerGroups ; optr<oend ; optr++ )
                out += matches[*optr].rm_so == -1 ? OOString( OONull ) :
                OOString( input+matches[*optr].rm_so,
                         (NSUInteger)(matches[*optr].rm_eo-matches[*optr].rm_so) );

		return out;
	}

	oo_inline OOStringArray parse( cOOString str ) const {
		OOPoolIfRequired;
		const char *input = str;
		OOStringArray out;

		regmatch_t matches[100];
		if ( exec( input, sizeof matches/sizeof matches[0], matches ) )
			out = matchGroups( input, matches );

		return out;
	}
	oo_inline OOStringArrayArray parseAll( cOOString str ) const {
		OOPoolIfRequired;
		const char *input = str;
        OOStringArrayArray out;

		regmatch_t matches[100];
		for ( ; exec( input, sizeof matches/sizeof matches[0], matches ) ; input += matches[0].rm_eo )
            out += matchGroups( input, matches );

		return out;
	}
	oo_inline OOString blockReplace( cOOString str, OOReplaceBlock callback ) const {
		OOPoolIfRequired;
		const char *input = str;
		OOString out;

		regmatch_t matches[100];
		for ( ; exec( input, sizeof matches / sizeof matches[0], matches ) ; input += matches[0].rm_eo )
            out += OOString( input, (NSUInteger)matches[0].rm_so )+callback( matchGroups( input, matches ) );

		return out += OOString( input );
	}
};

class OOReplace {
	regmatch_t dmatches[100];
	OOPattern pattern;
	OOString replace;
	int dollarCount;

    oo_inline OOPattern *dollarPattern() const {
        static OOPattern *dollarPattern;
		if ( !dollarPattern )
			dollarPattern = new OOPattern( "\\\\\\$|\\$[[:digit:]]+|\\${[[:digit:]]+}" );
        return dollarPattern;
    }

    void expandDollars( const char *input, const regmatch_t matches[100], const char *dstr, OOString &out ) const {
        regoff_t pos = 0;
        out += OOString( input, (NSUInteger)matches[0].rm_so );

        for ( int d=0 ; d<dollarCount ; d++ ) {
            out += OOString( dstr+pos, (NSUInteger)(dmatches[d].rm_so-pos) );
            const char *vptr = dstr+dmatches[d].rm_so+1;
            int v = atoi( *vptr == '{' ? vptr+1 : vptr );
            out += *vptr == '$' ? OOString( vptr, 1 ) :
            OOString( input+matches[v].rm_so, (NSUInteger)(matches[v].rm_eo-matches[v].rm_so) );
            pos = dmatches[d].rm_eo;
        }

        out += OOString( dstr+pos );
    }

public:
	oo_inline OOReplace() {}
	oo_inline OOReplace( const char *expr ) {
		init( expr );
	}
	oo_inline OOReplace( cOOString expr ) {
		init( expr );
	}
	oo_inline OOReplace( cOOString pat, cOOString rep, int flags = OO_REG_FLAGS ) {
		init( pat, rep, flags );
	}

	oo_inline void init( cOOString expr ) {
		OOPool pool;
		OOStringArray split = expr / expr[NSMakeRange(0,1)];
		int flags = OO_REG_FLAGS;

		for ( const char *options = *split[3] ; *options ; options++ )
			switch ( *options ) {
				case 'i': flags |= REG_ICASE; break;
				case 'm': flags |= REG_NEWLINE; break;
			}

		init( *split[1], *split[2], flags );
	}
	oo_inline void init( cOOString pat, cOOString rep, int flags = OO_REG_FLAGS ) {
		pattern.init( pat, flags );
		OOPool pool;
        if ( !!rep )
            dollarCount = dollarPattern()->find( replace <<= rep, sizeof dmatches / sizeof dmatches[0], dmatches );
	}

	oo_inline OOString exec( const char *input ) const {
		OOPoolIfRequired;
		const char *dstr = replace;
		regmatch_t matches[100];
		OOString out;

		for ( ; pattern.exec( input, sizeof matches / sizeof matches[0], matches ) ; input += matches[0].rm_eo )
            expandDollars( input, matches, dstr, out );

		return out += OOString( input );
	}
	oo_inline OOString exec( const char *input, cOOStringArray outputs ) {
		OOPoolIfRequired;
		regmatch_t matches[100];
		OOString out;

		for ( int i=0 ;
             pattern.exec( input, sizeof matches / sizeof matches[0], matches ) && i < outputs ;
             input += matches[0].rm_eo, i++ ) {
            const char *dstr = outputs[i];
            dollarCount = dollarPattern()->find( dstr, sizeof dmatches / sizeof dmatches[0], dmatches );
            expandDollars( input, matches, dstr, out );
		}

		return out += OOString( input );
	}

};

/**
 Initialisation for OOPattern class. Replaces \[wWdWsS] with character classes
 for compatability with perl. All compiled reguar expressions are cached.
 */

static OOArray<OOPointer<regex_t *> > cache;

inline void OOPattern::init( cOOString patin, int flags ) {
    if ( !patin )
        OOWarn( @"OOPattern::init - nil pattern passed" );

    pat <<= patin.get();

	if ( !!cache[flags][patin] ) {
		regex = *cache[flags][patin];
		return;
	}

#ifndef REG_ENHANCED
    OOPool pool;
	static BOOL patfix;

	if ( !patfix ) {
		static const char *shortcuts[] = {
			"w/$1[[:alnum:]]/", "W/$1[^[:alnum:]]/",
			"d/$1[[:digit:]]/", "D/$1[^[:digit:]]/",
			"s/$1[[:space:]]/", "S/$1[^[:space:]]/"};
		static int nfixes = sizeof shortcuts/sizeof *shortcuts;
		static OOReplace patternShortcuts[sizeof shortcuts/sizeof *shortcuts];

        static int donePatterns;
		if ( !patternShortcuts || !donePatterns++ ) {
			int savefix = patfix;
			patfix = YES;
			for ( int i=0 ; i<nfixes ; i++ )
				patternShortcuts[i].init( OOString( @"/(^|[^\\\\](\\\\\\\\)*)\\\\" )+shortcuts[i] );
			patfix = savefix;
		}

		for ( int i=0 ; i<nfixes ; i++ )
			pat = patternShortcuts[i].exec( pat );
	}
#endif

	regex = new regex_t;
	int error = regcomp( regex, pat, flags );
	if ( error ) {
		char errbuff[1024];
		regerror( error, regex, errbuff, sizeof errbuff );
		OOWarn( @"OOPattern::init() Regex compilation error: %s, in pattern: \"%@\"", errbuff, *pat );
		delete regex;
        regex = NULL;
	}
	else
		cache[flags][patin] = regex;
}

#endif