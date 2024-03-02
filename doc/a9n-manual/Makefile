TARGET := a9n-manual.pdf

PAGEDIR := ./pages
BUILDDIR := ./build

PAGE := $(shell find $(PAGEDIR) -type f -name "main.typ")

.PHONY : all watch clean

all : $(BUILDDIR)/$(TARGET)

$(BUILDDIR)/$(TARGET) : $(PAGE)
	typst compile $^ $@ 

watch : $(PAGE)
	typst watch $^ $(BUILDDIR)/$(TARGET)

clean :
	rm -rf $(BUILDDIR)/*
