TARGET := a9n-manual.pdf

ROOTDIR := ./
PAGEDIR := ./pages
COMPONENTDIR := ./components
RESOURCEDIR := ./resources
BUILDDIR := ./build

# entry point
MAIN_PAGE := $(shell find $(ROOTDIR) -type f -name "main.typ")

# normally pages
PAGES := $(shell find $(PAGEDIR) -type f -name "*.typ")
COMPONENTS := $(shell find $(COMPONENTDIR) -type f -name "*.typ")
RESOURCES := $(shell find $(RESOURCEDIR) -type f -name "*.typ")

.PHONY : all watch clean open

all : $(BUILDDIR)/$(TARGET)

$(BUILDDIR)/$(TARGET) : $(MAIN_PAGE) $(PAGES) $(COMPONENTS) $(RESOURCES)
	typst compile $< $@ --root $(ROOTDIR)

watch : $(MAIN_PAGE)
	typst watch $^ $(BUILDDIR)/$(TARGET)

clean :
	rm -rf $(BUILDDIR)/*

open :
	open $(BUILDDIR)/$(TARGET)
