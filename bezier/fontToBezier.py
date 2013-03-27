#!/usr/bin/python

import re
from math import sqrt

groups_per_glyph = 2
#maxgroups = 32
maxgroups = 64
g_scalefactor = 16

def ptadd(a,b):
    return (a[0]+b[0],a[1]+b[1])
def ptsub(a,b):
    return (a[0]-b[0],a[1]-b[1])
def middle(a,b):
    return ((a[0]+b[0])/2, (a[1]+b[1])/2)
def ptscale(k,a):
    return (a[0]*k, a[1]*k)
    

def makebezier(s):
    startpt = (0,0)
    lastpt = (0,0)
    lastctrlpt = None
    beziers = []
    tokenized = []
    cmd = None
    global pushed
    pushed = None
    
 #   it = re.finditer("(([a-zA-Z])|(-?[0-9.]+))([, \t\n]*)",s)
    it = re.finditer("(([a-zA-Z])|(-?[0-9.e-]+))([, \t\n]*)",s)

    def getpt():
        global pushed
        try:
            t=pushed or it.next()
            pushed = None
#            print t.groups()
            return float(t.groups()[2])
        except:         
            print t.groups()
            raise 
            raise Exception("error parsing svg path: float expected")
    def getpair():
        return (getpt(),getpt())

    try:
        while True:
            n = it.next()
#            print n.groups()
            a = n.groups()[1]
            if a: 
                cmd = a # otherwise: use last cmd
            else:
                pushed = n
                cmd = lastcmd
                if cmd=="m":
                    cmd = "l"
#                raise Exception("reusing last command not supported yet")
#            print cmd
            # Mmhvlqtz
            if cmd=="M": # move
                startpt = getpair()
                lastpt = startpt
            elif cmd=="m":
                startpt = ptadd(lastpt, getpair())
                lastpt = startpt
            elif cmd=="H": # horizline
                cur = (getpt(),lastpt[1])
                beziers.append((lastpt, middle(lastpt, cur), cur))
                lastpt = cur
            elif cmd=="h": # horizline
                delta = (getpt(),0)
                cur = ptadd(lastpt, delta)
                beziers.append((lastpt, middle(lastpt, cur), cur))
                lastpt = cur
            elif cmd=="v": # vertline
                delta = (0,getpt())
                cur = ptadd(lastpt, delta)
                beziers.append((lastpt, middle(lastpt, cur), cur))
                lastpt = cur
            elif cmd=="L": # line
                cur = getpair()
                beziers.append((lastpt, middle(lastpt, cur), cur))
                lastpt = cur
            elif cmd=="l": # line
                delta = getpair()
                cur = ptadd(lastpt, delta)
                beziers.append((lastpt, middle(lastpt, cur), cur))
                lastpt = cur
            elif cmd=="q": # quadratic bezier
                ctrlpt = ptadd(lastpt, getpair())
                endpt = ptadd(lastpt, getpair())
                lastctrlpt = ctrlpt
                beziers.append((lastpt, ctrlpt, endpt))
                lastpt = endpt
            elif cmd=="t": # smooth bezier, reusing a control point
                if not lastctrlpt:
                    lastctrlpt = lastpt
                endpt = ptadd(lastpt, getpair())
                ctrlpt = ptsub(ptadd(lastpt, lastpt),lastctrlpt)
                lastctrlpt = ctrlpt
                beziers.append((lastpt, ctrlpt, endpt))
                lastpt = endpt
            elif cmd=="c": # cubic bezier
                ctrlpt_a = ptadd(lastpt, getpair())
                ctrlpt_b = ptadd(lastpt, getpair())
                endpt = ptadd(lastpt, getpair())
                ctrlpt = middle(
                        ptscale( -0.5, ptadd(lastpt, endpt) ),
                        ptscale( 1.5, ptadd(ctrlpt_a, ctrlpt_b) ) )
                lastctrlpt = ctrlpt_b
                beziers.append((lastpt, ctrlpt, endpt))
                lastpt = endpt
            elif cmd=="s": # smooth cubic bezier, reusing a control point
                if not lastctrlpt:
                    lastctrlpt = lastpt
                ctrlpt_a = ptsub(ptadd(lastpt, lastpt),lastctrlpt)
                ctrlpt_b = ptadd(lastpt, getpair())
                endpt = ptadd(lastpt, getpair())
                ctrlpt = middle(
                        ptscale( -0.5, ptadd(lastpt, endpt) ),
                        ptscale( 1.5, ptadd(ctrlpt_a, ctrlpt_b) ) )
                lastctrlpt = ctrlpt_b
                beziers.append((lastpt, ctrlpt, endpt))
                lastpt = endpt

            elif cmd=="z" or cmd=="Z": # close path
                if lastpt!=startpt: # only emit extra bezier if it hasn't length 0
                    beziers.append((lastpt, middle(lastpt, startpt), startpt))
                lastpt = startpt
            else:
                raise Exception('unimplemented command: "%s"'%cmd)
            

            if cmd not in "qQtTcsCS":
                lastctrlpt = None # clear last control point
            lastcmd = cmd
    except StopIteration:
        return beziers
    except:
        print n.groups(), cmd
        raise


def convert_svg_font(filename, varbase="font_foo", whichchars=None, scale=1.0, offset=(0,0)):
    from xml.etree.ElementTree import ElementTree
    tree = ElementTree()
    tree.parse(filename)
    print tree.getroot().items()
#    if whichchars:
#        whichchars = unicode(whichchars)

    # FUCK YOU XML!!!!!!!!!!!!!!!!!!!!
    try:
        ns = re.match("\{.*\}",tree.find(".").tag).group(0)
    except:
        ns=""
    print ns
    font=tree.find("%sdefs/%sfont"%(ns,ns))
    glyphs=font.findall("%sglyph"%ns)
    default_horiz_adv = font.get("horiz-adv-x")

    # data stored in here
    horiz_advs = []
    bezierdata = []
    char = []

    # run through glyphs
    for g in glyphs:
        c = g.get("unicode")
        if not c:
            continue
        if (not whichchars) or (c in whichchars):
            h=g.get("horiz-adv-x")
            if not h:
                h=default_horiz_adv # yes, it does occur
            horiz_advs.append(int(h))
            d = g.get("d")
            if d:
                bezierdata.append(makebezier(d))
            else:
                bezierdata.append([]) # handle empty glyphs (like " ")
            char.append(c)

#    for b in bezierdata:
#        print calc_bbox(b)
    calc_bbox([item for sublist in bezierdata for item in sublist], scale, offset)

    # write out data to headerfile
    fd=file("%s.c"%varbase,"wb")
    fd.write('#include "../fontheader.h"\n\n')
    
    # data
    fd.write("const bezier_t %s_data[] = {\n"%varbase)
    first=True
    for b1 in bezierdata:
        for b in b1:
            if not first:
                fd.write(",\n")
            first=False
            a=tuple(map(lambda x:int(x*g_scalefactor*scale), (b[0][0]+offset[0],b[0][1]+offset[1],b[1][0]+offset[0],b[1][1]+offset[1],b[2][0]+offset[0],b[2][1]+offset[1])))
            fd.write("\t{%s, %s, %s, %s, %s, %s}"%a)
            if(max(a) > 32767):
                print "WARNING! max value exceeded"
            if(max(a) < -32768):
                print "WARNING! min value exceeded"
    fd.write("};\n\n")

    # glyphs
    fd.write("const glyph_t %s_glyph[] = {\n"%varbase)
    first=True
    cumlen=0;
    for i in range(len(bezierdata)):
        if not first:
            fd.write(",\n")
        first=False
        c=char[i]
        if c in "'\\":
            c="'\\%s'"%c
        else:
            c="'%s'"%c
        fd.write("\t{%s, %s, &%s_data[%s], %s, " % (c, horiz_advs[i]*g_scalefactor*scale, varbase, cumlen, len(bezierdata[i])))
        groupdata = sort_glyph_to_groups(bezierdata[i])
        if groupdata:
            starts, lens, pathlens = zip(*groupdata)
        else:
            starts, lens, pathlens = [],[], []
        pathlens = map(lambda x: int(x*g_scalefactor*scale), pathlens)
        fd.write(list_to_C(starts, maxgroups)+", "+list_to_C(lens, maxgroups)+","+"}")


        cumlen += len(bezierdata[i])

    fd.write("};\n\n")

    fd.write("const int %s_pathlens[] = "%varbase)
    lens = []
    for b1 in bezierdata:
        for b in b1:
            lens.append(int(sqrt((b[1][0]-b[0][0])**2+(b[1][1]-b[0][1])**2+(b[2][0]-b[1][0])**2+(b[2][1]-b[1][1])**2) * g_scalefactor*scale))
    fd.write(list_to_C(lens))
    fd.write(";\n\n")

    
def calc_bbox(bezierdata, scale, offset):
    b_min = [1000000, 1000000]
    b_max = [-1000000, -1000000]
    for b in bezierdata:
        for p in b:
            b_min[0] = min(b_min[0], p[0])
            b_min[1] = min(b_min[1], p[1])
            b_max[0] = max(b_max[0], p[0])
            b_max[1] = max(b_max[1], p[1])
#    b_min.extend(b_max)
#    return b_min
    b_min = ptadd(ptscale(scale, b_min), offset)
    b_max = ptadd(ptscale(scale, b_max), offset)
    print "X: %s..%s    Y: %s..%s"%(b_min[0], b_max[0], b_min[1], b_max[1])
            


def convert_svg(filename, varbase, groupcount, scale=1.0, offset=(0,0), flip=False):
    from xml.etree.ElementTree import ElementTree
    tree = ElementTree()
    tree.parse(filename)
#    if whichchars:
#        whichchars = unicode(whichchars)

    # FUCK YOU XML!!!!!!!!!!!!!!!!!!!!
    ns = re.match("\{.*\}",tree.find(".").tag).group(0)

    paths=tree.findall("%sg/%spath"%(ns,ns))
    bezierdata = []
    for path in paths:
        d = path.get("d")
        bezierdata.extend(makebezier(d))

    calc_bbox(bezierdata, scale, offset)


    # write out data to headerfile
    fd=file("%s.c"%varbase,"wb")
    fd.write('#include "../fontheader.h"\n\n')
    
    # data
    fd.write("const bezier_t %s_data[] = {\n"%varbase)
    first=True
    for b in bezierdata:
        if not first:
            fd.write(",\n")
        first=False
        if flip:
            a=tuple(map(lambda x:int(x*g_scalefactor*scale ), (b[0][0]+offset[0],-b[0][1]+offset[1],b[1][0]+offset[0],-b[1][1]+offset[1],b[2][0]+offset[0],-b[2][1]+offset[1])))
        else:
            a=tuple(map(lambda x:int(x*g_scalefactor*scale ), (b[0][0]+offset[0],b[0][1]+offset[1],b[1][0]+offset[0],b[1][1]+offset[1],b[2][0]+offset[0],b[2][1]+offset[1])))
        fd.write("\t{%s, %s, %s, %s, %s, %s}"%a)
        if(max(a) > 32767):
            print "WARNING! max value exceeded"
        if(max(a) < -32768):
            print "WARNING! min value exceeded"
    fd.write("};\n\n")

    # glyphs
    fd.write("const glyph_t %s_glyph = "%varbase)
    first=True
    cumlen=0;
    fd.write("{%s, %s, &%s_data[0], %s, " % ("' '", 0, varbase, len(bezierdata)))
    groupdata = sort_glyph_to_groups(bezierdata, groupcount)
    if groupdata:
        starts, lens, pathlens = zip(*groupdata)
    else:
        starts, lens, pathlens = [],[], []
    pathlens = map(lambda x: int(x*g_scalefactor*scale), pathlens)
    fd.write(list_to_C(starts, maxgroups)+", "+list_to_C(lens, maxgroups)+","+"}")



    fd.write(";\n\n")


def sort_glyph_to_groups(beziers, groupcount = groups_per_glyph):
    # TODO: calculate lens, figure out contiguous groups, then separate into groups_per_glyph groups.
    # returns a tuple of group-lengths
    
    if len(beziers)==0:
        return([])

    # calculate lengths
    lens=[]
    for b in beziers:
        lens.append(sqrt((b[1][0]-b[0][0])**2+(b[1][1]-b[0][1])**2+(b[2][0]-b[1][0])**2+(b[2][1]-b[1][1])**2))

    totallen = sum(lens)
    targetlen = totallen/groupcount
    
    # find connected paths
    start = 0
    groups=[]
    grouplens = []
    sumlen=0
    for i in range(len(beziers)-1):
        sumlen += lens[i]
        if sumlen >= targetlen*0.8 or \
                beziers[i][2]!=beziers[i+1][0]: # not connected
            # write out group
            groups.append((start, i-start+1, sumlen))
            grouplens.append(sumlen)
            start = i+1
            sumlen=0
    # last path
    sumlen += lens[-1]
    groups.append((start, len(beziers)-start, sumlen))
    grouplens.append(sumlen)

#    for l in grouplens:
#        print l/targetlen
#    print "splits: %s"%len(groups)

    #FIXME: the last group of a connected path has a tendency to be rather short
    #NOTE: groups_per_glyph is not strictly respected

    return(groups)

        


def list_to_C(l, fill=None):
    l = list(l) # cast if necessary
    if fill:
        if len(l) > fill:
            raise Exception("Array too long, got a length of %s, max is %s"%(len(l),fill))
        l = (l + [0]*fill)[0:fill]
    l = map(str, l)
    s = ", ".join(l)
    s = "{"+s+"}"
    return s

    


    





if __name__=="__main__":
#    b = makebezier(a)
#    s = bezier_to_header(b,1024)
#    fd = file("font.h","wb")
#    fd.write("int glyph_a[][6]=")
#    fd.write(s)
#    fd.close()
#    print s

    chars = "".join(map(chr, range(0x20,0x80)))
    convert_svg_font("Enriqueta-Regular.svg", "font_enri", chars, 0.2)
    convert_svg_font("Geo-Oblique.svg", "font_geo", chars, 0.2)
    convert_svg_font("progbot.svg", "font_led", chars, 0.2)
    convert_svg("revision_logo_lines.svg", "revision_logo", 20, 0.1,(0,0))
    convert_svg("rocket.svg", "rocket", 20, 1,(-93,-400))
#    convert_svg("forcer1.svg", "monkey", 20, flip=True)
