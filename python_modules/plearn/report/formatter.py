# formatter.py
# Copyright (C) 2006 Christian Dorion
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#   3. The name of the authors may not be used to endorse or promote
#      products derived from this software without specific prior written
#      permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
#  NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#  This file is part of the PLearn library. For more information on the PLearn
#  library, go to the PLearn Web site at www.plearn.org

# Author: Christian Dorion
import os, sys
from plearn.pyplearn.plearn_repr import plearn_repr

DEFAULT_WRITER = sys.stdout.write

def elemFormat(e):
    if isinstance(e, str):
        return e # Not quoted...
    elif isinstance(e, float):
        return "%.2f"%e
    return plearn_repr(e)

def twikiTableLine(table_line, elem_format=None, writer=DEFAULT_WRITER):    
    format = lambda e : (elem_format and elem_format%e) or elemFormat(e)
    writer('|  ')
    for element in table_line:
        writer("%s  |  " % format(element))
    writer("\n")

def twikiTable(table, headers=[], writer=DEFAULT_WRITER):
    writer( '%TABLE{ sort="off" tableborder="0" '
            'cellpadding="2" cellspacing="1" headerbg="#000099" headercolor="#FFFFCC" '
            'databg="#C8CB8F, #DBDDB5" headerrows="1" footerrows="0" }%\n' )

    if headers:
        twikiTableLine(headers, "*%s*", writer)

    for line in table:
        twikiTableLine(line, writer=writer)

def latexTable(table, headers=[], 
               align="", super_headers=[],
               padding=0.5, vpadding=0.0, caption="", label="",
               fontsize="", landscape=False, targetpos="", writer=DEFAULT_WRITER):
    lwriter = lambda line : writer("%s\n"%line)
    if align:
        assert len(align)==len(table[0]), \
               "%d != %d --- %s"%(len(align), len(table[0]), table[0])
    else:
        align = "c"*len(table[0])

    if padding:
        vpad = ""
        if vpadding > 0:
            vpad = r"\raisebox{%.3fcm}{\rule{0pt}{%.3fcm}}"%(-0.5*vpadding, vpadding)
            
        padded = r"%s@{\hspace{%.3fcm}%s}"%(align[0],padding,vpad)        
        for a in align[1:-1]:
            padded += "%s@{\\hspace{%.3fcm}}"%(a,2*padding)
        if len(align) > 1:
            padded += "%s@{\\hspace{%.3fcm}}"%(align[-1],padding)
        align = padded    

    if landscape:
        lwriter(r"\begin{landscape}")
        lwriter(r"\addtocounter{@inlandscapetable}{1}")
    lwriter("\\begin{table}%s"%targetpos)
    
    lwriter("\\begin{center}")
    if fontsize:
        lwriter(fontsize)
    lwriter("\\begin{tabular}{%s}"%align)
    
    if super_headers:
        writer("  ")
        latexTableLine(super_headers, writer)

    if headers:
        writer("  ")
        latexTableLine(headers, writer)
        lwriter("\\hline\\hline")
    else:
        lwriter("\\hline")
 
    for line in table:
        writer("  ")
        if isinstance(line, str):
            lwriter(line) # Single string is wrote as is...
        else:
            latexTableLine(line, writer)
    lwriter("\\hline")
       
    lwriter("\\end{tabular}")
    lwriter("\\end{center}")
    if caption:
        lwriter("    \\tabcaption{%s}"%caption)
    if label:
        lwriter("    \\label{table:%s}"%label)

    lwriter("\\end{table}")                
    if landscape:
        lwriter(r"\addtocounter{@inlandscapetable}{-1}")
        lwriter(r"\end{landscape}")

def latexTableLine(line, writer=DEFAULT_WRITER):
    endl = r"\\"
    handling_multicol = [] # For \multicolumn...
    for elem in line: 
        if elem is None:
            assert handling_multicol \
                and ( handling_multicol[-1].find("multicol") != -1
                      or handling_multicol[-1].find("hline") != -1 )
        elif elem == "NOENDL":
            endl = ""
        else:
            handling_multicol.append(elem)            
    writer('&'.join(handling_multicol) + endl + "\n")

def vpaddingLine(vpadding, length):
    vpad = r"\raisebox{%.3fcm}{\rule{0pt}{%.3fcm}}"%(-0.5*vpadding, vpadding)
    return [vpad]+[""]*(length-1)

def strictlyUpperTriangularTable(table, headers=[], format="%s"):
    """Returns a table of strings and modified headers suitable for latex/twikiTable.

    The 'table' is assumed to be a square matrix of numbers in which the
    subdiagonal AND diagonal elements are to be neglected. It can also be a
    pair of tables, in which case the format is assumed to handles pairs.
    """
    N = len(table)
    rows = iter(table)
    columns = lambda row: iter(row)
    if isinstance(table, tuple):
        table, subtable = table
        N = len(table)
        rows = iter( zip(table,subtable) )
        columns = lambda row: iter( zip(row[0], row[1]) )        
    assert N==len(table[0])

    formatted = []
    for i, row in enumerate(rows):
        if i == N-1:
            break
        
        formatted_row = [ ]
        if headers:
            formatted_row.append(headers[i])
            
        for j, col in enumerate( columns(row) ):
            if j == 0:
                continue
            elif j <= i:
                formatted_row.append('-'),
            else:
                formatted_row.append( format%col )
        formatted.append(formatted_row)

    return formatted, ['']+headers[1:]

def sideBySideTables(tables, headers, shared_headers):
    """Puts tables side by side in a single table.

    Tables are to be provided as a list ('tables') and 'headers' must be a
    list of lists of headers. The shared headers will be written only once
    at the begging of the merged table.
    """
    rows = len(tables[0])    
    columns = len(headers[0])
    shared_columns = len(shared_headers)
    
    merged_table = []
    for r in range(rows):
        row = tables[0][r]
        if isinstance(row, str): # Single string is wrote as is...
            merged_table.append(row)
            continue
        
        merged_row = list(row[:shared_columns])
        for table in tables:
            row = table[r]            
            merged_row.extend( row[shared_columns:] )
            merged_row.append("")
        assert merged_row.pop()=="" # Don't add empty col after last table
        merged_table.append(merged_row)

    merged_headers = list(shared_headers)
    for hdrs in headers:
        merged_headers.extend(hdrs[shared_columns:])
        merged_headers.append("")
    assert merged_headers.pop()=="" # Don't add empty col after last table

    return merged_table, merged_headers
        
#####  PDF creator  #########################################################

TEX_BEGIN = r"""
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Automatically generated by the 'mytex' program
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\documentclass[11pt]{article}
\usepackage{apstat_article_style}
\usepackage{mcgill_cover_classical}

\usepackage{lscape}

\include{MathStuff}
\include{LabelsAndRefs}
\newcommand{\prgname}[1]{\texttt{#1}}
\newcommand{\citeP}[1]{(\citeNP{#1})} %% Since I overwrite \cite...
\renewcommand{\cite}[1]{\citeN{#1}}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\renewcommand{\tabcaption}[1]{%
  \refstepcounter{table}%
  {\raggedright\hspace{0pt}\small\it 
      \captionheadfont\caparrowup%
      \tablename~\mbox{\arabic{table}}.~%
      \captionbodyfont{#1}%
  }}%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\begin{document}
"""

TEX_END = r"""

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\cleardoublepage
\end{document}
"""

def createPDF(file_name, content):
    assert file_name.endswith('.tex')    
    if isinstance(content, list):
        content = '\n'.join(content)
    
    tex_file = open(file_name, 'w')
    tex_file.write(TEX_BEGIN)
    tex_file.write(content)
    tex_file.write(TEX_END)
    tex_file.close()

    pdf_name = file_name.replace(".tex", '.pdf')
    os.system("rm -f %s"%pdf_name)
    os.system("pdflatex -interaction=nonstopmode %s >& /dev/null"%file_name)
    assert os.path.exists(pdf_name), "PDF could not be created!"
    os.system("pdflatex %s >& /dev/null"%file_name)
    os.system("pdflatex %s >& /dev/null"%file_name)
