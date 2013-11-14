CHECK_REGISTRY = $(top_builddir)/tests/check/test-registry.reg

REGISTRY_ENVIRONMENT =						\
	GST_REGISTRY=$(CHECK_REGISTRY)				\
	GST_PLUGIN_PATH=$(top_builddir)/src:$(GST_PLUGINS_DIR)

TESTS_ENVIRONMENT =						\
	CK_DEFAULT_TIMEOUT=20					\
	$(REGISTRY_ENVIRONMENT)

# run any given test by running make test.check
# if the test fails, run it again at at least debug level 2
%.check: %
	@$(TESTS_ENVIRONMENT)					\
	$*

CLEANFILES = core.* test-registry.*

$(CHECK_REGISTRY):
	$(TESTS_ENVIRONMENT)

TESTS =	$(check_PROGRAMS)

check_PROGRAMS =						\
	elements/dtlssrtp

elements_dtlssrtp_CFLAGS = $(GST_CFLAGS) $(GST_CHECK_CFLAGS) $(GIO_CFLAGS) -Werror \
				-DCERTTOOL_TEMPLATE=\"$(abs_top_builddir)/tests/certkey.tmpl\"
elements_dtlssrtp_LDADD = $(GST_LIBS) $(GST_CHECK_LIBS) $(GIO_LIBS)
