/*
   AGBCOMP compatible LZSS compressor
   Compresses files in a format that the GBA BIOS can decode.

   Original code by Haruhiko Okumura, 4/6/1989.
   12-2-404 Green Heights, 580 Nagasawa, Yokosuka 239, Japan.

   Anonymous, December 2001:
     Converted I/O to load the entire file into memory and operate
     on buffers.
     Modified to output in the GBA BIOS's native format.

   Damian Yerrick, July 2002:
     Translated the program into C from C++.
     Removed use of non-ANSI <sys/stat.h> that some compilers
     don't support.
     Made messages more informational.
     Corrected an off-by-one error in the allocation of the
     output size.
     Removed comp.h.  Now it's one file.

   Damian Yerrick, December 2002:
     Prevented matches that reference data before start of file.
     Began to standardize comments.

   Use, distribute, and modify this code freely.

   "I do not accept responsibility for any effects, adverse or otherwise, 
    that this code may have on you, your computer, your sanity, your dog, 
    and anything else that you can think of. Use it at your own risk."
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* set to 1 to receive diagnostics for matches
   that reference data before start of file */
#define DIAGNOSTIC_ILLEGAL_BACKWARD 0

/* Binary search tree functions */
void InitTree(void);
void InsertNode(int r);
void DeleteNode(int p);

/* Misc Functions */
void CompressLZ77(void);
int InChar(void);


/* Define information for compression
   (dont modify from 4096/18/2 if AGBCOMP format is required) */
#define N               4096   /* size of ring buffer (12 bit) */
#define F                 18   /* upper limit for match_length */
#define THRESHOLD          2   /* encode string into position and length
                                  if matched length is greater than this */
#define NIL                N   /* index for root of binary search trees */
#define TEXT_BUF_CLEAR     0   /* byte to initialize the area before text_buf with */

/* Compressor global variables.  If you actually want to USE this
   code in a non-trivial app, put these global variables in a struct,
   as the Allegro library did.
*/
unsigned long int codesize = 0;  // code size counter
// ring buffer of size N with extra F-1 bytes to facilitate string comparison
unsigned char text_buf[N + F - 1];
int match_position;  // global string match position
int match_length;  // global string match length
int lson[N + 1], rson[N + 257], dad[N + 1];  // left & right children & parents -- These constitute binary search trees.

unsigned char *InBuf, *OutBuf;
int InSize, OutSize, InOffset;



int main(int argc, char *argv[])
{
  FILE *InFile, *OutFile;  /* input & output files */

  if((argc != 3) ||
     (!strcmp(argv[1], "-?")) ||
     (!strcmp(argv[1], "--help")) )
  {
    fputs("lzgb by Damian Yerrick\n"
          "Compresses data into the Game Boy Advance's LZSS format.\n"
          "usage: lzgb <in filename> <out filename>\n", stderr);
    return 1;
  }

  /* Open input file */
  InFile = fopen(argv[1], "rb");
  if(!InFile) 
  {
    fputs("lzgb could not open ", stderr);
    perror(argv[1]);
    return EXIT_FAILURE;
  }

  /* Find the size of the input file and the worst-case output file */
  fseek(InFile, 0, SEEK_END);
  InSize = ftell(InFile);
  fseek(InFile, 0, SEEK_SET);
  OutSize = InSize + (InSize >> 3) + 16;

  /* Make a block of memory */
  InBuf = (unsigned char *)malloc(InSize);
  OutBuf = (unsigned char *)malloc(OutSize); 
  if(!InBuf || !OutBuf)
  {
    perror("lzgb could not allocate memory to hold the file");
    fclose(InFile);
    return EXIT_FAILURE;
  }
  if(fread(InBuf, sizeof(unsigned char), InSize, InFile) < InSize)
  {
    fputs("lzgb could not read ", stderr);
    perror(argv[1]);
    free(InBuf);
    free(OutBuf);
    fclose(InFile);
    return EXIT_FAILURE;
  }
  fclose(InFile);

  /* Compress the data */
  CompressLZ77();
  free(InBuf);
  
  /* Write output file */
  OutFile = fopen(argv[2], "wb");
  if(!OutFile)
  {
    fputs("lzgb could not open ", stderr);
    perror(argv[2]);
    free(OutBuf);
    return EXIT_FAILURE;
  }
  if(fwrite(OutBuf, sizeof(unsigned char), OutSize, OutFile) < OutSize)
  {
    fputs("lzgb could not write to ", stderr);
    perror(argv[2]);
    fclose(OutFile);
    free(OutBuf);
    return EXIT_FAILURE;
  }
  fclose(OutFile);

  /* All done. */
  free(OutBuf);
  printf("Compressed %d bytes to %d bytes\n", InSize, OutSize);
  return 0;
}


/* InitTree() **************************
   Initialize a binary search tree.

   For i = 0 to N - 1, rson[i] and lson[i] will be the right and
   left children of node i.  These nodes need not be initialized.
   Also, dad[i] is the parent of node i.  These are initialized
   to NIL (= N), which stands for 'not used.'
   For i = 0 to 255, rson[N + i + 1] is the root of the tree
   for strings that begin with character i.  These are
   initialized to NIL.  Note there are 256 trees.
*/
void InitTree(void)
{
  int  i;

  for (i = N + 1; i <= N + 256; i++)
    rson[i] = NIL;
  for (i = 0; i < N; i++)
    dad[i] = NIL;
}


/* InsertNode() ************************
   Inserts string of length F, text_buf[r..r+F-1], into one of the
   trees (text_buf[r]'th tree) and returns the longest-match position
   and length via the global variables match_position and match_length.
   If match_length = F, then removes the old node in favor of the new
   one, because the old one will be deleted sooner.
   Note r plays double role, as tree node and position in buffer.
*/
void InsertNode(int r)
{
  int  i, p, cmp;
  unsigned char  *key;

  cmp = 1;  key = &text_buf[r];  p = N + 1 + key[0];
  rson[r] = lson[r] = NIL;  match_length = 0;
  for( ; ; )
  {
    if(cmp >= 0)
    {
      if(rson[p] != NIL)
        p = rson[p];
      else
      {
        rson[p] = r;
        dad[r] = p;
        return;
      }
    }
    else
    {
      if(lson[p] != NIL)
        p = lson[p];
      else
      {
        lson[p] = r;
        dad[r] = p;
        return;
      }
    }
    for(i = 1; i < F; i++)
      if((cmp = key[i] - text_buf[p + i]) != 0)
        break;
    if(i > match_length)
    {
      match_position = p;
      if((match_length = i) >= F)
        break;
    }
  }
  dad[r] = dad[p];
  lson[r] = lson[p];
  rson[r] = rson[p];
  dad[lson[p]] = r;
  dad[rson[p]] = r;
  if(rson[dad[p]] == p)
    rson[dad[p]] = r;
  else
    lson[dad[p]] = r;
  dad[p] = NIL;  /* remove p */
}


/* DeleteNode() ************************
   Deletes node p from the tree.
*/
void DeleteNode(int p)  
{
  int  q;
  
  if(dad[p] == NIL)
    return;  /* not in tree */
  if(rson[p] == NIL)
    q = lson[p];
  else if(lson[p] == NIL)
    q = rson[p];
  else
  {
    q = lson[p];
    if(rson[q] != NIL)
    {
      do {
        q = rson[q];
      } while(rson[q] != NIL);
      rson[dad[q]] = lson[q];
      dad[lson[q]] = dad[q];
      lson[q] = lson[p];
      dad[lson[p]] = q;
    }
    rson[q] = rson[p];
    dad[rson[p]] = q;
  }
  dad[q] = dad[p];
  if (rson[dad[p]] == p) rson[dad[p]] = q;  else lson[dad[p]] = q;
  dad[p] = NIL;
}


/* CompressLZ77() **********************
   Compress InBuffer to OutBuffer.
*/
void CompressLZ77(void)
{
    int  i, c, len, r, s, last_match_length, code_buf_ptr;
    unsigned char  code_buf[17];
    unsigned short mask;
    unsigned char *FileSize;
    unsigned int curmatch;
    unsigned int savematch;
    unsigned int outbuf_so_far = 0;
    
    OutSize=4;  /* skip the compression type and file size */
    InOffset=0;
    curmatch=N-F;

    InitTree();  /* initialize trees */
    code_buf[0] = 0;  /* code_buf[1..16] saves eight units of code, and
        code_buf[0] works as eight flags, "1" representing that the unit
        is an unencoded letter (1 byte), "0" a position-and-length pair
        (2 bytes).  Thus, eight units require at most 16 bytes of code. */
    code_buf_ptr = 1;
    s = 0;  r = N - F;

    /* Clear the buffer */
    for (i = s; i < r; i++)
      text_buf[i] = TEXT_BUF_CLEAR;
    for (len = 0; len < F && (c = InChar()) != -1; len++)
        text_buf[r + len] = c;  /* Read F bytes into the last F bytes of
            the buffer */
    if (len == 0) return;  /* text of size zero */
    for (i = 1; i <= F; i++) InsertNode(r - i);  /* Insert the F strings,
        each of which begins with one or more 'space' characters.  Note
        the order in which these strings are inserted.  This way,
        degenerate trees will be less likely to occur. */
    InsertNode(r);  /* Finally, insert the whole string just read.  The
        global variables match_length and match_position are set. */

    /* GBA LZSS masks are big-endian */
    mask = 0x80;
    do {
        if (match_length > len) 
        {
          // match_length may be spuriously long near the end of text.
          match_length = len;  
        }

	/* If the match is not long enough, or it goes past the
	   beginning of the compressed data, send one byte. */
        if (match_length <= THRESHOLD ||
            ((curmatch - match_position) & 4095) > outbuf_so_far)
        {
#if DIAGNOSTIC_ILLEGAL_BACKWARD
          if(((curmatch - match_position) & 4095) > outbuf_so_far)
          {
            printf("Caught reference to %u at %u\n",
                    (curmatch - match_position) & 4095,
                    outbuf_so_far);
          }
#endif

          match_length = 1;
          code_buf[0] |= mask;  /* 'send one byte' flag */
          code_buf[code_buf_ptr++] = text_buf[r];  /* Send uncoded. */
          curmatch++;
          curmatch&=4095;
        } 
        else 
        {
          /* Send position and length pair. Note match_length > THRESHOLD. */
          
          // 0 byte is 4:length and 4:top 4 bits of match_position
          savematch=((curmatch-match_position)&4095)-1;
          curmatch+=match_length;
          curmatch&=4095;
          code_buf[code_buf_ptr++] = ((unsigned char)((savematch>>8)&0xf))
                        | ((match_length - (THRESHOLD + 1))<<4);

          code_buf[code_buf_ptr++] = (unsigned char) savematch;
  
        
        }
        outbuf_so_far += match_length;
        /* Shift mask left one bit. */
        if ((mask >>= 1) == 0) 
        {  
          /* Send remaining code.
            (GBA likes it inverted, with 0 for literal and 1 for match.) */
          code_buf[0]=~code_buf[0];
          for (i = 0; i < code_buf_ptr; i++)  /* Send at most 8 units of code */
          {
            OutBuf[OutSize++]=code_buf[i];
          }
          
          codesize += code_buf_ptr;
          code_buf[0] = 0;  
          code_buf_ptr = 1;
          mask = 0x80;
        }

        last_match_length = match_length;
        
        for (i = 0; i < last_match_length &&
                (c = InChar()) != -1; i++) 
        {
          DeleteNode(s);      /* Delete old strings and */
          text_buf[s] = c;    /* read new bytes */
          if (s < F - 1) text_buf[s + N] = c;  /* If the position is
              near the end of buffer, extend the buffer to make
              string comparison easier. */
          s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
              /* Since this is a ring buffer, increment the position
                 modulo N. */
          InsertNode(r);  /* Register the string in text_buf[r..r+F-1] */
        }

      
        while (i++ < last_match_length) 
        {    
            /* After the end of text, */
            DeleteNode(s);                  /* no need to read, but */
            s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
            if (--len) InsertNode(r);        /* buffer may not be empty. */
        }

    } while (len > 0);    /* until length of string to be processed is zero */

  
    
    if (code_buf_ptr > 1) 
    {     
      /* Send remaining code.
         (GBA likes it inverted, with 0 for literal and 1 for match.) */
      code_buf[0]=~code_buf[0];
      for (i = 0; i < code_buf_ptr; i++) 
      {
        OutBuf[OutSize++]=code_buf[i]; 
      }
      codesize += code_buf_ptr;
    }

    FileSize=(unsigned char*)OutBuf;
    FileSize[0]=0x10;
    FileSize[1]=((InSize>>0)&0xff);
    FileSize[2]=((InSize>>8)&0xff);
    FileSize[3]=((InSize>>16)&0xff);
}


/* InChar() ****************************
   Get the next character from the input stream, or -1 for end of file.
*/
int InChar()
{
  int Ret=0;
  unsigned char Input;

  Input=InBuf[InOffset++];
  if (InOffset>InSize) Ret=-1;
  else Ret=Input;
  
  return Ret;
}








