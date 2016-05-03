#!/usr/bin/env python
"""
define a category
"""
import branches
class Category:
    def __init__(self, name, cut, branch=branches.M4L):
        self.name = name
        self.branch = branch
        self.cut = "("+cut+"&&"+self.branch.get_cut_str()+")"

    def __str__(self):
        return "{name}".format(**self.__dict__)

    def __repr__(self):
        return "{__class__.__name__}(name={name!r}, cut={cut!r}, branch={branch!r})".\
                format(__class__=self.__class__, **self.__dict__)


# define some common categories
ggF_4mu_13TeV = Category(
    name="ggF_4mu_13TeV",
    cut="event_type==0",
)
ggF_2mu2e_13TeV = Category(
    name="ggF_2mu2e_13TeV",
    cut="event_type==2",
)
ggF_2e2mu_13TeV = Category(
    name="ggF_2e2mu_13TeV",
    cut="event_type==3",
)
ggF_4e_13TeV = Category(
    name="ggF_4e_13TeV",
    cut="event_type==1",
)

if __name__ == "__main__":
    print "Hello "+ str(ggF_4mu_13TeV)
    print repr(ggF_4mu_13TeV)

