SUBDIRS = src include parser_handlers benchmark	doc_example
ACLOCAL_AMFLAGS = -I m4

pcfiles = liborcus-@ORCUS_API_VERSION@.pc
if BUILD_SPREADSHEET_MODEL
pcfiles += liborcus-spreadsheet-model-@ORCUS_API_VERSION@.pc
endif

pkgconfig_DATA = $(pcfiles)
pkgconfigdir = $(libdir)/pkgconfig

doc_data = \
    doc/conf.py \
    doc/doxygen.conf \
    doc/index.rst \
    doc/cli/index.rst \
    doc/cli/orcus_csv.rst \
    doc/cli/orcus_gnumeric.rst \
    doc/cli/orcus_json.rst \
    doc/cli/orcus_ods.rst \
    doc/cli/orcus_xls_xml.rst \
    doc/cli/orcus_xlsx.rst \
    doc/cli/orcus_xml.rst \
    doc/cli/orcus_yaml.rst \
    doc/cpp/filter/index.rst \
    doc/cpp/index.rst \
    doc/cpp/model/index.rst \
    doc/cpp/model/json.rst \
    doc/cpp/model/spreadsheet.rst \
    doc/cpp/model/yaml.rst \
    doc/cpp/parser/csv.rst \
    doc/cpp/parser/index.rst \
    doc/cpp/parser/json.rst \
    doc/cpp/parser/util.rst \
    doc/cpp/parser/xml.rst \
    doc/cpp/parser/yaml.rst \
    doc/overview/index.rst \
    doc/overview/json.rst \
    doc/python/index.rst

doc_example_data = \
    doc_example/files/document.ods \
    doc_example/files/multi-sheets.ods

bin_data = \
    bin/env-osx.sh \
    bin/orcus-common-func.sh \
    bin/orcus-css-dump.sh \
    bin/orcus-csv.sh \
    bin/orcus-detect.sh \
    bin/orcus-gnumeric.sh \
    bin/orcus-json.sh \
    bin/orcus-mso-encryption.sh \
    bin/orcus-ods.sh \
    bin/orcus-test.sh \
    bin/orcus-xls-xml.sh \
    bin/orcus-xlsx.sh \
    bin/orcus-xml-dump.sh \
    bin/orcus-xml.sh \
    bin/orcus-yaml.sh \
    bin/orcus-zip-dump.sh \
    bin/run-python.sh

misc_data = \
    misc/OpenDocument-v1.2-os-schema.rng \
    misc/gnumeric.xsd \
    misc/xls-xml-tokens.txt \
    misc/gen-odf-tokens.py \
    misc/ooxml-ecma-376/OfficeOpenXML-XMLSchema.zip \
    misc/ooxml-ecma-376/OpenPackagingConventions-XMLSchema.zip \
    misc/gen-gnumeric-tokens.py \
    misc/gen-tokens.py \
    misc/gen-ooxml-tokens.py \
    misc/generate-tokens.sh \
    misc/dump-xsd-keys.py \
    misc/token_util.py

slickedit_data = \
    slickedit/token-gen.vpj \
    slickedit/orcus.vpw \
    slickedit/external-headers.vpj \
    slickedit/orcus.vpj

test_data = \
    test/css/basic10.css \
    test/css/basic11.css \
    test/css/basic12.css \
    test/css/basic13.css \
    test/css/basic14.css \
    test/css/basic1.css \
    test/css/basic2.css \
    test/css/basic3.css \
    test/css/basic4.css \
    test/css/basic5.css \
    test/css/basic6.css \
    test/css/basic7.css \
    test/css/basic8.css \
    test/css/basic9.css \
    test/css/chained1.css \
    test/css/chained2.css \
    test/css/complex/callout.css \
    test/css/complex/excel-html.css \
    test/css/test.css \
    test/csv/double-quotes/check.txt \
    test/csv/double-quotes/input.csv \
    test/csv/normal-quotes/check.txt \
    test/csv/normal-quotes/input.csv \
    test/csv/quoted-with-delim/check.txt \
    test/csv/quoted-with-delim/input.csv \
    test/csv/simple-numbers/check.txt \
    test/csv/simple-numbers/input.csv \
    test/csv/split-sheet/check-1.txt \
    test/csv/split-sheet/check-2.txt \
    test/csv/split-sheet/check-3.txt \
    test/csv/split-sheet/input.csv \
    test/gnumeric/raw-values-1/check.txt \
    test/gnumeric/raw-values-1/input.gnumeric \
    test/gnumeric/test.gnumeric \
    test/json/basic1/check.txt \
    test/json/basic1/input.json \
    test/json/basic2/check.txt \
    test/json/basic2/input.json \
    test/json/basic3/check.txt \
    test/json/basic3/input.json \
    test/json/basic4/check.txt \
    test/json/basic4/input.json \
    test/json/nested1/check.txt \
    test/json/nested1/input.json \
    test/json/nested2/check.txt \
    test/json/nested2/input.json \
    test/json/refs1/check.txt \
    test/json/refs1/input.json \
    test/json/refs1/ref.json \
    test/json/swagger/check.txt \
    test/json/swagger/input.json \
    test/ods/borders/grid-box.ods \
    test/ods/borders/single-cells.ods \
    test/ods/column-width-row-height/input.ods \
    test/ods/date-cell/input.ods \
    test/ods/formatted-text/bold-and-italic.ods \
    test/ods/formula-1/check.txt \
    test/ods/formula-1/input.ods \
    test/ods/formula-2/check.txt \
    test/ods/formula-2/input.ods \
    test/ods/japanese.ods \
    test/ods/raw-values-1/check.txt \
    test/ods/raw-values-1/input.ods \
    test/ods/styles/cell-styles.xml \
    test/ods/styles/number-format.xml \
    test/ods/test.ods \
    test/python/csv_export.py \
    test/python/csv.py \
    test/python/file_load_common.py \
    test/python/gnumeric.py \
    test/python/json.py \
    test/python/module.py \
    test/python/ods.py \
    test/python/perf/test_json.py \
    test/python/xls_xml.py \
    test/python/xlsx.py \
    test/xlsx/background-color/standard.xlsx \
    test/xlsx/boolean-values/check.txt \
    test/xlsx/boolean-values/input.xlsx \
    test/xlsx/borders/colors.xlsx \
    test/xlsx/borders/directions.xlsx \
    test/xlsx/borders/grid-box.xlsx \
    test/xlsx/borders/single-cells.xlsx \
    test/xlsx/column-width-row-height/input.xlsx \
    test/xlsx/data-table/multi-table.xlsx \
    test/xlsx/data-table/one-variable.xlsx \
    test/xlsx/date-cell/input.xlsx \
    test/xlsx/date-time/input.xlsx \
    test/xlsx/doc-structure/unordered-sheet-positions.xlsx \
    test/xlsx/empty-shared-strings/check.txt \
    test/xlsx/empty-shared-strings/input.xlsx \
    test/xlsx/formatted-text/bold-and-italic.xlsx \
    test/xlsx/formatted-text/colored-text.xlsx \
    test/xlsx/formula-array-1/check.txt \
    test/xlsx/formula-array-1/input.xlsx \
    test/xlsx/formula-cells/check.txt \
    test/xlsx/formula-cells/input.xlsx \
    test/xlsx/formula-shared/check.txt \
    test/xlsx/formula-shared/input.xlsx \
    test/xlsx/formula-simple.xlsx \
    test/xlsx/hidden-rows-columns/input.xlsx \
    test/xlsx/matrix-results/input.xlsx \
    test/xlsx/merged-cells/simple.xlsx \
    test/xls-xml/background-color/standard.xml \
    test/xls-xml/basic/check.txt \
    test/xls-xml/basic/input.xml \
    test/xls-xml/bold-and-italic/check.txt \
    test/xls-xml/bold-and-italic/input.xml \
    test/xls-xml/borders/colors.xml \
    test/xls-xml/borders/directions.xml \
    test/xls-xml/borders/grid-box.xml \
    test/xls-xml/borders/single-cells.xml \
    test/xls-xml/character-set/input.xml \
    test/xls-xml/colored-text/check.txt \
    test/xls-xml/colored-text/input.xml \
    test/xls-xml/column-width-row-height/input.xml \
    test/xls-xml/date-time/input.xml \
    test/xls-xml/empty-rows/check.txt \
    test/xls-xml/empty-rows/input.xml \
    test/xls-xml/formula-array-1/check.txt \
    test/xls-xml/formula-array-1/input.xml \
    test/xls-xml/formula-cells-1/check.txt \
    test/xls-xml/formula-cells-1/input.xml \
    test/xls-xml/formula-cells-2/check.txt \
    test/xls-xml/formula-cells-2/config.yaml \
    test/xls-xml/formula-cells-2/input.xml \
    test/xls-xml/hidden-rows-columns/input.xml \
    test/xls-xml/leading-whitespace/check.txt \
    test/xls-xml/leading-whitespace/input.xml \
    test/xls-xml/matrix-results/input.xml \
    test/xls-xml/merged-cells/check.txt \
    test/xls-xml/merged-cells/input.xml \
    test/xls-xml/named-expression/check.txt \
    test/xls-xml/named-expression/input.xml \
    test/xls-xml/named-expression-sheet-local/check.txt \
    test/xls-xml/named-expression-sheet-local/input.xml \
    test/xls-xml/raw-values-1/check.txt \
    test/xls-xml/raw-values-1/input.xml \
    test/xls-xml/table/autofilter.xml \
    test/xls-xml/text-alignment/input.xml \
    test/xls-xml/view/cursor-per-sheet.xml \
    test/xls-xml/view/cursor-split-pane.xml \
    test/xls-xml/view/frozen-pane.xml \
    test/xlsx/named-expression/check.txt \
    test/xlsx/named-expression/input.xlsx \
    test/xlsx/named-expression-sheet-local/check.txt \
    test/xlsx/named-expression-sheet-local/input.xlsx \
    test/xlsx/pivot-table/chart-simple.xlsx \
    test/xlsx/pivot-table/error-values.xlsx \
    test/xlsx/pivot-table/group-by-dates.xlsx \
    test/xlsx/pivot-table/group-by-numbers.xlsx \
    test/xlsx/pivot-table/group-field.xlsx \
    test/xlsx/pivot-table/many-fields.xlsx \
    test/xlsx/pivot-table/mixed-type-field.xlsx \
    test/xlsx/pivot-table/three-pivot-tables-on-one-sheet.xlsx \
    test/xlsx/pivot-table/two-pivot-caches.xlsx \
    test/xlsx/pivot-table/two-tables-one-source.xlsx \
    test/xlsx/raw-values-1/check.txt \
    test/xlsx/raw-values-1/input.xlsx \
    test/xlsx/revision/cell-change-basic.xlsx \
    test/xlsx/table/autofilter-text-filter-1.xlsx \
    test/xlsx/table/autofilter.xlsx \
    test/xlsx/table/table-1.xlsx \
    test/xlsx/table/table-2.xlsx \
    test/xlsx/test.xlsx \
    test/xlsx/text-alignment/input.xlsx \
    test/xlsx/view/cursor-per-sheet.xlsx \
    test/xlsx/view/cursor-split-pane.xlsx \
    test/xlsx/view/frozen-pane.xlsx \
    test/xml/bom/check.txt \
    test/xml/bom/input.xml \
    test/xml/cdata-1/check.txt \
    test/xml/cdata-1/input.xml \
    test/xml/custom-decl-1/check.txt \
    test/xml/custom-decl-1/input.xml \
    test/xml/default-ns/check.txt \
    test/xml/default-ns/input.xml \
    test/xml/doctype/html.xml \
    test/xml/encoded-attrs/test1.xml \
    test/xml/encoded-char/check.txt \
    test/xml/encoded-char/input.xml \
    test/xml-mapped/attribute-basic/check.txt \
    test/xml-mapped/attribute-basic/input.xml \
    test/xml-mapped/attribute-basic/map.xml \
    test/xml-mapped/attribute-namespace/check.txt \
    test/xml-mapped/attribute-namespace/input.xml \
    test/xml-mapped/attribute-namespace/map.xml \
    test/xml-mapped/attribute-range-self-close/check.txt \
    test/xml-mapped/attribute-range-self-close/input.xml \
    test/xml-mapped/attribute-range-self-close/map.xml \
    test/xml-mapped/attribute-single-element-2/check.txt \
    test/xml-mapped/attribute-single-element-2/input.xml \
    test/xml-mapped/attribute-single-element-2/map.xml \
    test/xml-mapped/attribute-single-element/check.txt \
    test/xml-mapped/attribute-single-element/input.xml \
    test/xml-mapped/attribute-single-element/map.xml \
    test/xml-mapped/content-basic/check.txt \
    test/xml-mapped/content-basic/input.xml \
    test/xml-mapped/content-basic/map.xml \
    test/xml-mapped/content-namespace-2/check.txt \
    test/xml-mapped/content-namespace-2/input.xml \
    test/xml-mapped/content-namespace-2/map.xml \
    test/xml-mapped/content-namespace/check.txt \
    test/xml-mapped/content-namespace/input.xml \
    test/xml-mapped/content-namespace/map.xml \
    test/xml-mapped/fuel-economy/check.txt \
    test/xml-mapped/fuel-economy/input.xml \
    test/xml-mapped/fuel-economy/map.xml \
    test/xml/no-decl-1/check.txt \
    test/xml/no-decl-1/input.xml \
    test/xml/ns-alias-1/check.txt \
    test/xml/ns-alias-1/input.xml \
    test/xml/parse-only/rss/input.xml \
    test/xml/self-closing-root/check.txt \
    test/xml/self-closing-root/input.xml \
    test/xml/simple/check.txt \
    test/xml/simple/input.xml \
    test/xml/single-quote/check.txt \
    test/xml/single-quote/input.xml \
    test/xml-structure/attribute-1/check.txt \
    test/xml-structure/attribute-1/input.xml \
    test/xml-structure/basic-1/check.txt \
    test/xml-structure/basic-1/input.xml \
    test/xml-structure/basic-2/check.txt \
    test/xml-structure/basic-2/input.xml \
    test/xml-structure/basic-3/check.txt \
    test/xml-structure/basic-3/input.xml \
    test/xml-structure/namespace-default/check.txt \
    test/xml-structure/namespace-default/input.xml \
    test/xml-structure/nested-repeat-1/check.txt \
    test/xml-structure/nested-repeat-1/input.xml \
    test/xml/underscore-identifier/check.txt \
    test/xml/underscore-identifier/input.xml \
    test/yaml/basic1/input.yaml \
    test/yaml/basic2/input.yaml \
    test/yaml/basic3/input.yaml \
    test/yaml/boolean/input.yaml \
    test/yaml/empty-value-map-1/input.yaml \
    test/yaml/empty-value-map-2/input.yaml \
    test/yaml/empty-value-sequence-1/input.yaml \
    test/yaml/empty-value-sequence-2/input.yaml \
    test/yaml/invalids/1.yaml \
    test/yaml/invalids/2.yaml \
    test/yaml/invalids/3.yaml \
    test/yaml/literal-block-1/input.yaml \
    test/yaml/literal-block-2/input.yaml \
    test/yaml/map-key-1/input.yaml \
    test/yaml/multi-line-1/input.yaml \
    test/yaml/multi-line-2/input.yaml \
    test/yaml/null/input.yaml \
    test/yaml/quoted-string/input.yaml \
    test/yaml/swagger/input.yaml \
    test/yaml/url/input.yaml

EXTRA_DIST = \
	CHANGELOG \
	LICENSE \
	README.md \
	liborcus.pc.in \
	autogen.sh \
	$(bin_data) \
	$(doc_data) \
	$(doc_example_data) \
	$(misc_data) \
	$(slickedit_data) \
	$(test_data)

.PHONY: distclean-local dist-hook doc-doxygen doc-sphinx doc

distclean-local:
	rm -rf *.pc

dist-hook:
	git log --date=short --pretty="format:@%cd  %an  <%ae>  [%H]%n%n%s%n%n%e%b" | sed -e "s|^\([^@]\)|\t\1|" -e "s|^@||" >$(distdir)/ChangeLog


doc-doxygen:
	@echo "Building documentation by doxygen..."
	@cd doc && doxygen doxygen.conf

doc-sphinx:
	@echo "Building documentation by sphinx..."
	@sphinx-build -b html ./doc/ ./doc/_build

doc: doc-doxygen doc-sphinx



