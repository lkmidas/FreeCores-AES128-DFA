SUBDIRS = simulation

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

simulation:
	$(MAKE) -C $@

clean:
	@rm -vf fault.txt
	$(MAKE) clean -C simulation
